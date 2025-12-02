"use strict";
/*
 * Respond to commands over a websocket to access the beat-box program
 */

var fs       = require('fs');
var socketio = require('socket.io');
var io; 
var dgram    = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');
	
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});

	// Periodically poll the local beat-box for current state and broadcast
	// to all connected clients. This keeps clients in sync even if the
	// device state changes outside the web UI.
	setInterval(function() {
		// Relay but with no specific socket so replies are broadcast
		relayToLocalPort(null, 'get volume', 'volume-reply');
		relayToLocalPort(null, 'get tempo', 'tempo-reply');
		relayToLocalPort(null, 'get mode', 'mode-reply');
	}, 1000);
};

function handleCommand(socket) {
	console.log("Setting up socket handlers.");

	socket.on('read-uptime', function() {
		readAndSendFile(socket, '/proc/uptime', 'uptime-reply');
	});
	socket.on('mode', function(modeNumber) {
		console.log("Got mode command: " + modeNumber);
		relayToLocalPort(socket, "set drum mode " + modeNumber, "mode-reply");
	});
	socket.on('volume', function(volumeNumber) {
		console.log("Got volume command: " + volumeNumber);
		// UDP expects: "set volume <N>"
		relayToLocalPort(socket, "set volume " + volumeNumber, "volume-reply");
	});
	socket.on('tempo', function(tempoNumber) {
		console.log("Got tempo command: " + tempoNumber);
		// UDP expects: "set tempo <BPM>"
		relayToLocalPort(socket, "set tempo " + tempoNumber, "tempo-reply");
	});
	socket.on('play', function(songNumber) {
		console.log("Got play command: " + songNumber);
		if (songNumber === null || songNumber === undefined) return;
		// Map numeric IDs from the UI to drum sound names expected by BeatBox
		var name = String(songNumber);
		switch(name) {
		case '0': name = 'base'; break;   // Base -> base/bass
		case '1': name = 'hihat';  break;   // Hi-Hat
		case '2': name = 'snare'; break;  // Snare
		default:
			// allow textual names through unchanged
			name = String(songNumber);
		}
		console.log('Playing sound name:', name);
		// UDP play handler expects "play <name> sound"
		relayToLocalPort(socket, "play " + name + " sound", "play-reply");
	});
	socket.on('stop', function(notUsed) {
		console.log("Got stop command: ");
		relayToLocalPort(socket, "stop", "stop-reply");
	});

	// On new connection, query device for current volume so the web UI
	// initializes to the real device volume (prevents sudden jumps).
	// Keep automatic queries as a fallback (short delay) but also accept
	// explicit client requests below. The client will typically request
	// immediately after loading the page.
	setTimeout(function() { relayToLocalPort(socket, "get volume", "volume-reply"); }, 50);
	setTimeout(function() { relayToLocalPort(socket, "get tempo", "tempo-reply"); }, 120);

	// Client-initiated reads (web UI asks for current device state)
	socket.on('read-volume', function() {
		console.log('Client requested current volume');
		relayToLocalPort(socket, 'get volume', 'volume-reply');
	});
	socket.on('read-tempo', function() {
		console.log('Client requested current tempo');
		relayToLocalPort(socket, 'get tempo', 'tempo-reply');
	});
};

function readAndSendFile(socket, absPath, commandString) {
	fs.exists(absPath, function(exists) {
		if (exists) {
			fs.readFile(absPath, function(err, fileData) {
				if (err) {
					socket.emit("beatbox-error", 
							"ERROR: Unable to read file " + absPath);
				} else {
					// Don't send back empty files.
					if (fileData.length > 0) {
						socket.emit(commandString, fileData.toString('utf8'));;
					}
				}
			});
		} else {
			socket.emit("beatbox-error", 
					"ERROR: File " + absPath + " not found.");
		}
	});
}

function relayToLocalPort(socket, data, replyCommandName) {
	console.log('relaying to local port command: ' + data);
	
	// Info for connecting to the local process via UDP
	var PORT = 12345;	// Port of local application
	var HOST = '192.168.7.2';
	var buffer = new Buffer(data);

	// Send an error if we have not got a reply in a second
	var errorTimer = setTimeout(function() {
	 console.log("ERROR: No reply from local application.");
	 if (socket) socket.emit("beatbox-error", "SERVER ERROR: No response from beat-box application. Is it running?");
	 else io.sockets.emit("beatbox-error", "SERVER ERROR: No response from beat-box application. Is it running?");
	}, 1000);

	
	var client = dgram.createSocket('udp4');
	client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
	    if (err) 
	    	throw err;
	    console.log('UDP message sent to ' + HOST +':'+ PORT);
	});
	
	client.on('listening', function () {
	    var address = client.address();
	    console.log('UDP Client: listening on ' + address.address + ":" + address.port);
	});
	// Handle an incoming message over the UDP from the local application.
	client.on('message', function (message, remote) {
		console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
        
		var reply = message.toString('utf8')
		if (socket) {
			socket.emit(replyCommandName, reply);
		} else {
			// Broadcast to all connected sockets
			io.sockets.emit(replyCommandName, reply);
		}
		clearTimeout(errorTimer);
		client.close();
	});
	
	client.on("UDP Client: close", function() {
	    console.log("closed");
	});
	client.on("UDP Client: error", function(err) {
	    console.log("error: ",err);
	});	
}