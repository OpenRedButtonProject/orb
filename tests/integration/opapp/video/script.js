/**
 * HBBTV WebSocket Client
 */
class HBBTVWebSocketClient {
    // Status constants
    static STATUS_CONNECTING = 1;
    static STATUS_PRESENTING = 2;
    static STATUS_STOPPED = 3;
    static STATUS_ERROR = 0;

    // HBBTV Method constants
    static METHOD_SELECT_CHANNEL = "org.hbbtv.ipplayer.selectChannel";
    static METHOD_PLAY = "org.hbbtv.ipplayer.play";
    static METHOD_PAUSE = "org.hbbtv.ipplayer.pause";
    static METHOD_STOP = "org.hbbtv.ipplayer.stop";
    static METHOD_SEEK = "org.hbbtv.ipplayer.seek";
    static METHOD_RESUME = "org.hbbtv.ipplayer.resume";
    static METHOD_STATUS_UPDATE = "org.hbbtv.ipplayback.statusUpdate";
    static METHOD_MEDIA_POSITION_UPDATE = "org.hbbtv.ipplayback.mediaPositionUpdate";
    static METHOD_SET_COMPONENTS = "org.hbbtv.ipplayback.setComponents";
    static METHOD_NEGOTIATE_METHODS = "org.hbbtv.negotiateMethods";

    constructor(videoPlayer = null) {
        this.websocket = null;
        this.isConnected = false;
        this.messageId = 1;
        this.negotiated = false;
        this.supportedMethods = {
            terminalToApp: [],
            appToTerminal: []
        };
        this.logEntries = document.getElementById('logEntries');
        this.statusElement = document.getElementById('websocketStatus');

        // HBBTV Objects
        this.appManager = document.getElementById('app-manager');
        this.capabilities = document.getElementById('capabilities');
        this.videoPlayer = videoPlayer;

        this.initializeElements();
        this.bindEvents();
        this.log('HBBTV WebSocket Client initialized', 'websocket');
        this.initializeHBBTV();
    }

    getJsonRpcUrl() {
        if (!this.capabilities) return null;
        const xml = this.capabilities.xmlCapabilities;
        const elements = xml.getElementsByTagName('json_rpc_server');
        if (elements.length > 0 && elements[0].hasAttribute('url')) {
        return elements[0].getAttribute('url');
        }
        return null;
    }

    initializeHBBTV() {
        try {
            if (this.appManager) {
                this.log('HBBTV Application Manager found', 'success');
                this.log(`App Manager version: ${this.appManager.version}`, 'info');
            } else {
                this.log('HBBTV Application Manager not available', 'warn');
            }

            if (this.capabilities) {
                this.log('HBBTV Capabilities object found', 'success');
                this.logCapabilities();
            } else {
                this.log('HBBTV Capabilities object not available', 'warn');
            }
        } catch (error) {
            this.log(`HBBTV initialization error: ${error.message}`, 'error');
        }
    }

    logCapabilities() {
        try {
            if (this.capabilities && this.capabilities.xmlCapabilities) {
                const xml = this.capabilities.xmlCapabilities;
                this.log('HBBTV Capabilities XML loaded', 'success');

                // Log available capabilities
                const jsonRpcServer = this.getJsonRpcUrl();
                if (jsonRpcServer) {
                    this.log(`JSON-RPC Server URL: ${jsonRpcServer}`, 'info');
                }

                // Log other capabilities
                const elements = xml.getElementsByTagName('*');
                for (let i = 0; i < elements.length; i++) {
                    const element = elements[i];
                    if (element.tagName && element.textContent) {
                        this.log(`Capability: ${element.tagName} = ${element.textContent}`, 'info');
                    }
                }
            }
        } catch (error) {
            this.log(`Error reading capabilities: ${error.message}`, 'error');
        }
    }

    initializeElements() {
        this.defaultWebSocketUrl = this.getJsonRpcUrl();
    }

    bindEvents() {
        // No specific events to bind for HBBTVWebSocketClient itself,
        // as it's a client-side WebSocket handler.
    }


    log(message, level = 'info') {
        const timestamp = new Date().toLocaleTimeString();
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry ${level}`;
        logEntry.innerHTML = `<span class="timestamp">[${timestamp}]</span> 🔌 ${message}`;

        if (this.logEntries) {
            this.logEntries.appendChild(logEntry);
            this.logEntries.scrollTop = this.logEntries.scrollHeight;
        }

        console.log(`[${timestamp}] WebSocket: ${message}`);
    }

    updateStatus(message, type = 'disconnected') {
        if (this.statusElement) {
            this.statusElement.textContent = message;
            this.statusElement.className = `status ${type}`;
        }
    }

    connect() {
        const url = this.defaultWebSocketUrl;

        if (this.isConnected) {
            this.log('Already connected to WebSocket', 'warn');
            return;
        }

        this.log(`Connecting to WebSocket: ${url}`, 'websocket');
        this.updateStatus('Connecting...', 'connecting');

        try {
            this.websocket = new WebSocket(url);

            this.websocket.onopen = () => {
                this.isConnected = true;
                this.log('WebSocket connection established', 'success');
                this.updateStatus('Connected - Negotiating...', 'connected');
                this.sendNegotiation();
            };

            this.websocket.onmessage = (event) => {
                this.handleMessage(event.data);
            };

            this.websocket.onclose = (event) => {
                this.isConnected = false;
                this.log(`WebSocket connection closed: ${event.code} - ${event.reason}`, 'warn');
                this.updateStatus('Disconnected', 'disconnected');
            };

            this.websocket.onerror = (error) => {
                this.log(`WebSocket error: ${error}`, 'error');
                this.updateStatus('Connection Error', 'error');
                console.warn('WebSocket connection failed, but video player will continue to work');
            };

        } catch (error) {
            this.log(`Failed to create WebSocket connection: ${error.message}`, 'error');
            this.updateStatus('Connection Failed', 'error');
            console.warn('WebSocket initialization failed, but video player will continue to work');
        }
    }

    disconnect() {
        if (this.websocket && this.isConnected) {
            this.websocket.close(1000, 'Client disconnect');
            this.log('Disconnecting from WebSocket', 'websocket');
        }
    }

    handleMessage(data) {
        try {
            const message = JSON.parse(data);
            this.log(`Received: ${JSON.stringify(message, null, 2)}`, 'websocket');

            if (message.jsonrpc === "2.0") {
                if (message.result !== undefined) {
                    this.log(`RPC Result: ${JSON.stringify(message.result, null, 2)}`, 'success');

                    if (message.result.method === HBBTVWebSocketClient.METHOD_NEGOTIATE_METHODS) {
                        this.handleNegotiationResponse(message.result);
                    }
                } else if (message.error) {
                    this.log(`RPC Error: ${message.error.code} - ${message.error.message}`, 'error');
                } else if (message.method) {
                    this.log(`RPC Request from server: ${message.method}`, 'info');
                    this.handleServerRequest(message);
                }
            }
        } catch (error) {
            this.log(`Failed to parse message: ${error.message}`, 'error');
        }
    }

    sendNegotiation() {
        const params = {
            terminalToApp: [
                HBBTVWebSocketClient.METHOD_SELECT_CHANNEL,
                HBBTVWebSocketClient.METHOD_PLAY,
                HBBTVWebSocketClient.METHOD_PAUSE,
                HBBTVWebSocketClient.METHOD_STOP,
                HBBTVWebSocketClient.METHOD_SEEK,
                HBBTVWebSocketClient.METHOD_RESUME,
            ],
            appToTerminal: [
                HBBTVWebSocketClient.METHOD_STATUS_UPDATE,
                HBBTVWebSocketClient.METHOD_MEDIA_POSITION_UPDATE,
                HBBTVWebSocketClient.METHOD_SET_COMPONENTS
            ]
        };

        const message = {
            jsonrpc: "2.0",
            id: this.messageId++,
            method: HBBTVWebSocketClient.METHOD_NEGOTIATE_METHODS,
            params: params
        };

        this.log('Sending negotiation message', 'websocket');

        try {
            this.websocket.send(JSON.stringify(message));
            this.log('Negotiation message sent successfully', 'success');
        } catch (error) {
            this.log(`Failed to send negotiation message: ${error.message}`, 'error');
        }
    }

    handleNegotiationResponse(result) {
        this.negotiated = true;
        this.supportedMethods = {
            terminalToApp: result.terminalToApp || [],
            appToTerminal: result.appToTerminal || []
        };

        this.log('Negotiation completed successfully', 'success');
        this.updateStatus('Connected - Negotiated', 'connected');
    }

    handleServerRequest(message) {
        switch (message.method) {
            case HBBTVWebSocketClient.METHOD_PLAY:
                this.handlePlayRequest(message);
                break;
            case HBBTVWebSocketClient.METHOD_PAUSE:
                this.handlePauseRequest(message);
                break;
            case HBBTVWebSocketClient.METHOD_STOP:
                this.handleStopRequest(message);
                break;
            case HBBTVWebSocketClient.METHOD_SEEK:
                this.handleSeekRequest(message);
                break;
            case HBBTVWebSocketClient.METHOD_SELECT_CHANNEL:
                this.handleSelectChannelRequest(message);
                break;
            default:
                this.log(`Unknown server request method: ${message.method}`, 'warn');
                this.sendErrorResponse(message.id, -32601, 'Method not found');
        }
    }

    handlePlayRequest(message) {
        this.log('Handling play request from server', 'info');

        if (this.videoPlayer && this.videoPlayer.isInitialized) {
            this.videoPlayer.video.play().then(() => {
                this.log('Video playback started via server request', 'success');
                this.sendSuccessResponse(message.id, HBBTVWebSocketClient.METHOD_PLAY);
            }).catch(error => {
                this.log(`Failed to start playback: ${error.message}`, 'error');
                this.sendErrorResponse(message.id, -32000, 'Failed to start playback');
            });
        } else {
            this.log('Video player not initialized', 'error');
            this.sendErrorResponse(message.id, -32000, 'Video player not initialized');
        }
    }

    handlePauseRequest(message) {
        this.log('Handling pause request from server', 'info');

        if (this.videoPlayer && this.videoPlayer.isInitialized) {
            this.videoPlayer.video.pause();
            this.log('Video playback paused via server request', 'success');
            this.sendSuccessResponse(message.id, HBBTVWebSocketClient.METHOD_PAUSE);
        } else {
            this.log('Video player not initialized', 'error');
            this.sendErrorResponse(message.id, -32000, 'Video player not initialized');
        }
    }

    handleStopRequest(message) {
        this.log('Handling stop request from server', 'info');

        if (this.videoPlayer && this.videoPlayer.isInitialized) {
            this.videoPlayer.video.pause();
            this.videoPlayer.video.currentTime = 0;
            this.log('Video playback stopped via server request', 'success');
            this.sendSuccessResponse(message.id, HBBTVWebSocketClient.METHOD_STOP);
        } else {
            this.log('Video player not initialized', 'error');
            this.sendErrorResponse(message.id, -32000, 'Video player not initialized');
        }
    }

    handleSeekRequest(message) {
        this.log('Handling seek request from server', 'info');

        if (this.videoPlayer && this.videoPlayer.isInitialized) {
            if (message.params && typeof message.params.offset === 'number') {
                const offset = message.params.offset / 1000;
                this.videoPlayer.video.currentTime = offset;
                this.log('Video seek completed via server request', 'success');
                this.sendSuccessResponse(message.id, HBBTVWebSocketClient.METHOD_SEEK);
            } else {
                this.log('Invalid seek parameters: offset required', 'error');
                this.sendErrorResponse(message.id, -32602, 'Invalid params: offset required');
            }
        } else {
            this.log('Video player not initialized', 'error');
            this.sendErrorResponse(message.id, -32000, 'Video player not initialized');
        }
    }

    handleSelectChannelRequest(message) {
        this.log('Handling select channel request from server', 'info');

        if (message.params && message.params.url) {
            if (this.videoPlayer) {
                this.videoPlayer.defaultStreamUrl = message.params.url;
                this.videoPlayer.loadStream();
                this.log(`Channel selected: ${message.params.url}`, 'success');
                this.sendSuccessResponse(message.id, HBBTVWebSocketClient.METHOD_SELECT_CHANNEL);
            } else {
                this.log('Video player not available', 'error');
                this.sendErrorResponse(message.id, -32000, 'Video player not available');
            }
        } else {
            this.log('No URL provided in select channel request', 'error');
            this.sendErrorResponse(message.id, -32602, 'Invalid params: URL required');
        }
    }

    sendSuccessResponse(requestId, method) {
        const response = {
            jsonrpc: "2.0",
            result: { method: method },
            id: requestId
        };

        try {
            this.websocket.send(JSON.stringify(response));
            this.log('Success response sent successfully', 'success');
        } catch (error) {
            this.log(`Failed to send success response: ${error.message}`, 'error');
        }
    }

    sendErrorResponse(requestId, code, message) {
        const response = {
            jsonrpc: "2.0",
            error: { code: code, message: message },
            id: requestId
        };

        try {
            this.websocket.send(JSON.stringify(response));
            this.log('Error response sent successfully', 'success');
        } catch (error) {
            this.log(`Failed to send error response: ${error.message}`, 'error');
        }
    }
}

/**
 * HTML5 Video Player
 */
class HTML5VideoPlayer {
    constructor(websocketClient = null) {
        this.video = document.getElementById('videoPlayer');
        this.isInitialized = false;
        this.sessionID = 0;
        this.playStartTime = null;
        this.logEntries = document.getElementById('logEntries');
        this.websocketClient = websocketClient;

        this.initializeElements();
        this.bindEvents();
        this.log('HTML5 Video Player initialized', 'info');

        // Auto-load default stream
        setTimeout(() => {
            this.log('Auto-loading default stream...', 'info');
            this.loadStream();
        }, 500);
    }

    initializeElements() {
        this.defaultStreamUrl = 'http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4';
    }

    bindEvents() {

        // Video events
        this.video.addEventListener('loadstart', () => {
            this.log('Video load started', 'info');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_CONNECTING, 0);
        });
        this.video.addEventListener('play', () => {
            this.log('Video playback started', 'info');
            this.playStartTime = Date.now();
        });
        this.video.addEventListener('playing', () => {
            this.log('Video is playing', 'success');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_PRESENTING, 0);
        });
        this.video.addEventListener('pause', () => {
            this.log('Video playback paused', 'info');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_PRESENTING, 0);
        });
        this.video.addEventListener('ended', () => {
            this.log('Video playback ended', 'info');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_STOPPED, 0);
        });
        this.video.addEventListener('error', (e) => {
            const error = this.video.error;
            let errorMessage = 'Unknown error';
            let errorCode = 3;

            if (error) {
                switch(error.code) {
                    case MediaError.MEDIA_ERR_ABORTED:
                        errorMessage = 'Video playback aborted';
                        errorCode = 1;
                        break;
                    case MediaError.MEDIA_ERR_NETWORK:
                        errorMessage = 'Network error while loading video';
                        errorCode = 2;
                        break;
                    case MediaError.MEDIA_ERR_DECODE:
                        errorMessage = 'Video decoding error';
                        errorCode = 3;
                        break;
                    case MediaError.MEDIA_ERR_SRC_NOT_SUPPORTED:
                        errorMessage = 'Video format not supported';
                        errorCode = 4;
                        break;
                    default:
                        errorMessage = `Error code: ${error.code}`;
                        errorCode = 5;
                }
            }
            this.log(`Video error: ${errorMessage}`, 'error');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_ERROR, errorCode);
        });
        this.video.addEventListener('waiting', () => {
            this.log('Video waiting for data', 'warn');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_CONNECTING, 0);
        });
        this.video.addEventListener('stalled', () => {
            this.log('Video stalled', 'warn');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_CONNECTING, 0);
        });
        this.video.addEventListener('abort', () => {
            this.log('Video loading aborted', 'warn');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_STOPPED, 0);
        });
        this.video.addEventListener('emptied', () => {
            this.log('Video element emptied', 'info');
            this.sendStatusUpdate(HBBTVWebSocketClient.STATUS_STOPPED, 0);
        });
        this.video.addEventListener('timeupdate', () => {
            if (this.lastPositionUpdate === undefined ||
                (Date.now() - this.lastPositionUpdate) >= 5000) {
                this.sendMediaPositionUpdate();
                this.lastPositionUpdate = Date.now();
            }
        });
    }

    log(message, level = 'info') {
        const timestamp = new Date().toLocaleTimeString();
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry ${level}`;
        logEntry.innerHTML = `<span class="timestamp">[${timestamp}]</span> 🎥 ${message}`;

        if (this.logEntries) {
            this.logEntries.appendChild(logEntry);
            this.logEntries.scrollTop = this.logEntries.scrollHeight;
        }

        console.log(`[${timestamp}] Video: ${message}`);
    }


    loadStream() {
        const url = this.defaultStreamUrl;
        this.log(`Loading video stream: ${url}`, 'info');
        this.initializeVideo(url);
    }

    initializeVideo(url) {
        try {
            this.video.pause();
            this.video.currentTime = 0;
            this.sessionID++;
            this.video.src = url;
            this.video.load();

            this.isInitialized = true;
            this.log(`Video source set successfully (SessionID: ${this.sessionID})`, 'success');

            // Enable autoplay after video is loaded
            this.video.addEventListener('loadeddata', () => {
                this.log('Video data loaded, attempting autoplay...', 'info');
                this.attemptAutoplay();
            }, { once: true });

        } catch (error) {
            const errorMessage = error.message || error.toString() || 'Unknown error occurred';
            this.log(`Failed to load video: ${errorMessage}`, 'error');
        }
    }

    attemptAutoplay() {
        if (this.video.paused) {
            this.video.play().then(() => {
                this.log('Autoplay started successfully', 'success');
            }).catch(error => {
                this.log(`Autoplay failed: ${error.message}`, 'warn');
                // Try again with user interaction
                this.log('Autoplay blocked by browser. Click anywhere to start playback.', 'info');
                this.enableClickToPlay();
            });
        }
    }

    enableClickToPlay() {
        const clickHandler = () => {
            this.video.play().then(() => {
                this.log('Video started via user interaction', 'success');
            }).catch(error => {
                this.log(`Failed to start video: ${error.message}`, 'error');
            }).finally(() => {
                document.removeEventListener('click', clickHandler);
            });
        };

        document.addEventListener('click', clickHandler);
    }



    getStatusName(status) {
        switch(status) {
            case HBBTVWebSocketClient.STATUS_CONNECTING:
                return 'CONNECTING';
            case HBBTVWebSocketClient.STATUS_PRESENTING:
                return 'PRESENTING';
            case HBBTVWebSocketClient.STATUS_STOPPED:
                return 'STOPPED';
            case HBBTVWebSocketClient.STATUS_ERROR:
                return 'ERROR';
            default:
                return `UNKNOWN(${status})`;
        }
    }

    sendStatusUpdate(status, error = 0) {
        if (this.websocketClient && this.websocketClient.negotiated) {
            const params = {
                sessionID: this.sessionID,
                status: status,
                error: error
            };

            const message = {
                jsonrpc: "2.0",
                id: this.websocketClient.messageId++,
                method: HBBTVWebSocketClient.METHOD_STATUS_UPDATE,
                params: params
            };

            const statusName = this.getStatusName(status);
            this.websocketClient.log(`Auto-sending status update: ${statusName} (${status}) (error: ${error}, sessionID: ${this.sessionID})`, 'websocket');

            try {
                this.websocketClient.websocket.send(JSON.stringify(message));
                this.websocketClient.log('Status update message sent successfully', 'success');
            } catch (error) {
                this.websocketClient.log(`Failed to send status update: ${error.message}`, 'error');
            }
        }
    }

    sendMediaPositionUpdate() {
        if (this.websocketClient && this.websocketClient.negotiated && this.isInitialized) {
            const currentTime = this.video.currentTime * 1000;
            const playSpeed = this.video.playbackRate;

            let playPosition;
            if (this.playStartTime) {
                playPosition = this.playStartTime + currentTime;
            } else {
                playPosition = Date.now();
            }

            const params = {
                sessionID: this.sessionID,
                playPosition: Math.floor(playPosition),
                playSpeed: playSpeed,
                currentTimeShiftMode: 3,
                playbackOffset: Math.floor(currentTime),
                maxOffset: Math.floor(this.video.duration * 1000)
            };

            const message = {
                jsonrpc: "2.0",
                id: this.websocketClient.messageId++,
                method: HBBTVWebSocketClient.METHOD_MEDIA_POSITION_UPDATE,
                params: params
            };

            this.websocketClient.log(`Auto-sending media position update (sessionID: ${this.sessionID})`, 'websocket');

            try {
                this.websocketClient.websocket.send(JSON.stringify(message));
                this.websocketClient.log('Media position update message sent successfully', 'success');
            } catch (error) {
                this.websocketClient.log(`Failed to send media position update: ${error.message}`, 'error');
            }
        }
    }

    destroy() {
        if (this.video) {
            this.video.pause();
            this.video.src = '';
            this.video.load();
        }
        this.isInitialized = false;
        this.log('Player destroyed', 'info');
    }
}

// Initialize applications when page loads
document.addEventListener('DOMContentLoaded', () => {
    try {
        const websocketClient = new HBBTVWebSocketClient();
        const videoPlayer = new HTML5VideoPlayer(websocketClient);

        // Set up bidirectional references
        websocketClient.videoPlayer = videoPlayer;

        window.videoPlayer = videoPlayer;
        window.websocketClient = websocketClient;

        // Auto-connect to WebSocket
        setTimeout(() => {
            websocketClient.connect();
        }, 1000);

    } catch (error) {
        console.error('Error during initialization:', error);
    }
});

// Cleanup on page unload
window.addEventListener('beforeunload', () => {
    if (window.websocketClient) {
        window.websocketClient.disconnect();
    }
    if (window.videoPlayer) {
        window.videoPlayer.destroy();
    }
});
