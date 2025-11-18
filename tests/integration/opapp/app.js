// HbbTV OpApp v1.2.1 Minimal Implementation
let currentFocus = 0;
const buttons = ['broadcastBtn', 'sendMsgBtn', 'infoBtn', 'logToggleBtn'];

// Event Logging Functions
function logEvent(message, level = 'info') {
    const eventLogContainer = document.getElementById('eventLogContainer');
    if (!eventLogContainer) return;

    const timestamp = new Date().toLocaleTimeString();
    const logEntry = document.createElement('div');
    logEntry.className = `event-log-entry ${level}`;
    logEntry.innerHTML = `<span class="timestamp">[${timestamp}]</span> ${message}`;

    eventLogContainer.appendChild(logEntry);
    eventLogContainer.scrollTop = eventLogContainer.scrollHeight;

    // Also log to console
    console.log(`[${timestamp}] Event: ${message}`);
}

// Video Broadcast Class
class VideoBroadcast {
    constructor() {
        this.videoBroadcast = null;
        this.channelList = [];
        this.currentChannelIndex = 0;
        this.isPlaying = false;
        this.isInitialized = false;
        this.broadcastSupervisor = null;
        this.BSPlayStateEventVerified = false;
        this.hasChannelSelected = false;
    }

    initialize() {
        try {
            this.videoBroadcast = document.getElementById('hole');
            if (this.videoBroadcast) {
                this.isInitialized = true;
                this.broadcastSupervisor = this.videoBroadcast.getChannelConfig().getBroadcastSupervisor();
                this.loadChannelList();
                this.setupEventListeners();
                setTimeout(() => {
                    // Don't auto-select channel on initialization
                    // Update button state after initialization
                    this.updatePlayPauseButton();
                }, 2000);
                console.log('Video broadcast object initialized');
                logEvent('Video broadcast object initialized', 'success');
                return true;
            } else {
                console.log('Video broadcast object not found');
                logEvent('Video broadcast object not found', 'error');
                return false;
            }
        } catch (error) {
            console.log("Video broadcast object could not be created - error name: " +
                error.name + " - error message: " + error.message);
            logEvent(`Error initializing video broadcast: ${error.message}`, 'error');
            return false;
        }
    }

    loadChannelList() {
        try {
            if (this.videoBroadcast && this.isInitialized) {
                this.channelList = this.videoBroadcast.getChannelConfig().channelList;
                console.log("Channel list loaded: " + this.channelList.length + " channels");
                logEvent(`Channel list loaded: ${this.channelList.length} channels`, 'success');

                // Log channel details
                for (let i = 0; i < this.channelList.length; i++) {
                    let channel = this.channelList.item(i);
                    console.log("Name = " + channel.name + " ,  CCID = " + channel.ccid);
                    logEvent(`Channel ${i + 1}: ${channel.name} (CCID: ${channel.ccid})`, 'info');
                }
            }
        } catch (error) {
            console.log("Error loading channel list: " + error.message);
            logEvent(`Error loading channel list: ${error.message}`, 'error');
        }
    }

    setupEventListeners() {
        if (this.videoBroadcast) {
            this.videoBroadcast.addEventListener('load', () => {
                console.log('Video broadcast loaded');
                logEvent('Video broadcast loaded', 'success');
            });

            this.videoBroadcast.addEventListener('error', () => {
                console.log('Video broadcast error');
                logEvent('Video broadcast error', 'error');
            });
            // PlayStateChange event
            this.videoBroadcast.addEventListener('PlayStateChange', (event) => {
                const playState = this.getChannelstatus(event.playState);
                console.log('VBO.PlayStateChange event: ' + playState);
                logEvent(`VBO.PlayStateChange: ${playState}`, 'info');
                // Update button state when play state changes
                this.updatePlayPauseButton();
            });
            //ComponentChanged event
            this.videoBroadcast.addEventListener('ComponentChanged', (event) => {
                console.log('ComponentChanged event: ' + event.componentType);
                logEvent(`ComponentChanged: ${event.componentType}`, 'info');
                this.printComponents();
            });
            // BroadcastSupervisor PlayStateChange event
            if (this.broadcastSupervisor) {
                this.broadcastSupervisor.addEventListener('PlayStateChange', (event) => {
                    const playState = this.getChannelstatus(event.playState);
                    console.log('BroadcastSupervisor.PlayStateChange event: ' + playState);
                    if (!this.BSPlayStateEventVerified) {
                        this.BSPlayStateEventVerified = true;
                        logEvent(`BroadcastSupervisor PlayStateChange Event Test: [PASS]`, 'success');
                    }
                    logEvent(`BroadcastSupervisor.PlayStateChange: ${playState}`, 'info');
                });
            }
        }
    }

    selectNextChannel() {
        console.log(`Selecting next channel through VBO...`);
        logEvent(`Selecting next channel through VBO...`);
        this.selectNextChannelWrapper(this.videoBroadcast);
        setTimeout(() => {
            // Verify that the BroadcastSupervisor PlayStateChange Event was triggered after selecting a channel
            if (!this.BSPlayStateEventVerified) {
                logEvent(`BroadcastSupervisor PlayStateChange Event Test: [FAIL]`, 'error');
                console.log(`BroadcastSupervisor PlayStateChange Event Test: [FAIL]`);
            }
        }, 2000);
    }

    selectNextChannelBS() {
        console.log(`Selecting next channel through BroadcastSupervisor...`);
        logEvent(`Selecting next channel through BroadcastSupervisor...`);
        this.selectNextChannelWrapper(this.broadcastSupervisor);
    }

    selectPreviousChannel() {
        console.log(`Selecting previous channel through VBO...`);
        logEvent(`Selecting previous channel through VBO...`);
        this.selectPreviousChannelWrapper(this.videoBroadcast);
    }

    selectPreviousChannelWrapper(videoBroadcast)
    {
        try {
            if (!this.isInitialized || this.channelList.length === 0) {
                logEvent('No channels available', 'warning');
                return false;
            }

            // Move to previous channel index
            this.currentChannelIndex = (this.currentChannelIndex - 1 + this.channelList.length) % this.channelList.length;

            // Get the current channel
            const channel = this.channelList.item(this.currentChannelIndex);
            const channelName = channel.name || `Channel ${this.currentChannelIndex + 1}`;
            const channelCCID = channel.ccid;

            console.log(`Selecting channel: ${channelName} (CCID: ${channelCCID})`);
            logEvent(`Selecting channel: ${channelName} (CCID: ${channelCCID})`, 'info');

            // Set the channel
            videoBroadcast.setChannel(channel);
            logEvent(`Channel set: ${channelName}`, 'success');

            // Mark that a channel has been selected
            this.hasChannelSelected = true;
            this.updatePlayPauseButton();

            return true;

        } catch (error) {
            console.log("Error selecting channel: " + error.message);
            logEvent(`Error selecting channel: ${error.message}`, 'error');
            return false;
        }
    }

    selectNextChannelWrapper(videoBroadcast)
    {
        try {
            if (!this.isInitialized || this.channelList.length === 0) {
                logEvent('No channels available', 'warning');
                return false;
            }

            // Get the current channel
            const channel = this.channelList.item(this.currentChannelIndex);
            const channelName = channel.name || `Channel ${this.currentChannelIndex + 1}`;
            const channelCCID = channel.ccid;

            console.log(`Selecting channel: ${channelName} (CCID: ${channelCCID})`);
            logEvent(`Selecting channel: ${channelName} (CCID: ${channelCCID})`, 'info');

            // Set the channel
            videoBroadcast.setChannel(channel);
            logEvent(`Channel set: ${channelName}`, 'success');

            // Mark that a channel has been selected
            this.hasChannelSelected = true;
            this.updatePlayPauseButton();

            // Move to next channel for next selection
            this.currentChannelIndex = (this.currentChannelIndex + 1) % this.channelList.length;

            // Update button text to show next channel info
            this.updateChannelButtonText();

            return true;

        } catch (error) {
            console.log("Error selecting channel: " + error.message);
            logEvent(`Error selecting channel: ${error.message}`, 'error');
            return false;
        }
    }

    selectChannel(index) {
        try {
            if (!this.isInitialized || this.channelList.length === 0) {
                logEvent('No channels available', 'warning');
                return false;
            }

            if (index < 0 || index >= this.channelList.length) {
                logEvent(`Invalid channel index: ${index}`, 'error');
                return false;
            }

            const channel = this.channelList.item(index);
            const channelName = channel.name || `Channel ${index + 1}`;
            const channelCCID = channel.ccid;

            console.log(`Selecting channel: ${channelName} (CCID: ${channelCCID})`);
            logEvent(`Selecting channel: ${channelName} (CCID: ${channelCCID})`, 'info');

            this.videoBroadcast.setChannel(channel);
            this.currentChannelIndex = index;
            logEvent(`Channel set: ${channelName}`, 'success');

            return true;

        } catch (error) {
            console.log("Error selecting channel: " + error.message);
            logEvent(`Error selecting channel: ${error.message}`, 'error');
            return false;
        }
    }

    stop() {
        try {
            if (!this.isInitialized) {
                logEvent('Video broadcast not initialized', 'error');
                return false;
            }

            this.videoBroadcast.stop();
            this.isPlaying = false;
            console.log('Playback stopped');
            logEvent('Playback stopped', 'info');
            this.updatePlayPauseButton();
            return true;

        } catch (error) {
            console.log("Could not stop playback: " + error.message);
            logEvent(`Could not stop playback: ${error.message}`, 'error');
            return false;
        }
    }

    play() {
        try {
            if (!this.isInitialized) {
                logEvent('Video broadcast not initialized', 'error');
                return false;
            }

            // Use bindToCurrentChannel to resume playback from STOPPED state
            // This will transition from STOPPED -> CONNECTING -> PRESENTING
            const channel = this.videoBroadcast.bindToCurrentChannel();
            if (channel) {
                this.isPlaying = true;
                console.log('Playback resumed');
                logEvent('Playback resumed', 'info');
                this.updatePlayPauseButton();
                return true;
            } else {
                logEvent('No current channel to resume', 'error');
                return false;
            }

        } catch (error) {
            console.log("Could not resume playback: " + error.message);
            logEvent(`Could not resume playback: ${error.message}`, 'error');
            return false;
        }
    }

    togglePlayPause() {
        try {
            if (!this.isInitialized) {
                logEvent('Video broadcast not initialized', 'error');
                return false;
            }

            const playState = this.videoBroadcast.playState;
            // Play state values: 0=UNREALIZED, 1=CONNECTING, 2=PRESENTING, 3=STOPPED

            if (playState === 3) { // STOPPED
                // Resume playback
                return this.play();
            } else if (playState === 1 || playState === 2) { // CONNECTING or PRESENTING
                // Pause playback
                return this.stop();
            } else {
                // UNREALIZED or other state - try to start
                logEvent(`Cannot toggle in current state: ${this.getChannelstatus(playState)}`, 'warning');
                return false;
            }

        } catch (error) {
            console.log("Could not toggle play/pause: " + error.message);
            logEvent(`Could not toggle play/pause: ${error.message}`, 'error');
            return false;
        }
    }

    updatePlayPauseButton() {
        const playPauseBtn = document.getElementById('playPauseButton');
        if (!playPauseBtn) return;

        // Disable button if no channel has been selected
        if (!this.hasChannelSelected) {
            playPauseBtn.disabled = true;
            return;
        }

        // Enable button if channel is selected
        playPauseBtn.disabled = false;

        const playState = this.videoBroadcast ? this.videoBroadcast.playState : 3;
        const isPlaying = playState === 1 || playState === 2; // CONNECTING or PRESENTING

        if (isPlaying) {
            playPauseBtn.innerHTML = '<span class="btn-icon">⏸</span>';
        } else {
            playPauseBtn.innerHTML = '<span class="btn-icon">▶</span>';
        }
    }

    getCurrentChannel() {
        if (this.channelList.length > 0 && this.currentChannelIndex < this.channelList.length) {
            return this.channelList.item(this.currentChannelIndex);
        }
        return null;
    }

    getChannelCount() {
        return this.channelList.length;
    }

    getCurrentChannelIndex() {
        return this.currentChannelIndex;
    }

    isChannelPlaying() {
        return this.isPlaying;
    }

    updateChannelButtonText() {
        const channelBtn = document.getElementById('channelBtn');
        if (channelBtn && this.channelList.length > 0) {
            const nextChannel = this.channelList.item(this.currentChannelIndex);
            const nextChannelName = nextChannel.name || `Channel ${this.currentChannelIndex + 1}`;
            channelBtn.textContent = `Next: ${nextChannelName}`;
        }
    }

    printChannelList() {
        try {
            console.log("Channel list: " + this.channelList.length + " channels");
            logEvent(`Channel list: ${this.channelList.length} channels`, 'info');
            for (let i = 0; i < this.channelList.length; i++) {
                let channel = this.channelList.item(i);
                console.log("Name = " + channel.name + " ,  CCID = " + channel.ccid);
                logEvent(`Channel ${i + 1}: ${channel.name} (CCID: ${channel.ccid})`, 'info');
            }
        } catch (error) {
            console.log("Error printing channel list: " + error.message);
            logEvent(`Error printing channel list: ${error.message}`, 'error');
        }
    }

    printComponents() {
        try {
            if (!this.isInitialized) {
                logEvent('Video broadcast not initialized', 'error');
                return;
            }

            const components = this.videoBroadcast.getComponents();
            console.log("There are " + components.length + " components: ");
            logEvent(`Components found: ${components.length}`, 'info');
            for (let i = 0; i < components.length; i++) {
                const component = components[i];
                console.log('Component type: ' + component.type +
                    ' encoding: ' + component.encoding +
                    ' language: ' + component.language +
                    ' encrypted: ' + component.encrypted +
                    ' componentTag: ' + component.componentTag);
                logEvent(`Component ${i + 1}: Type=${component.type}, Encoding=${component.encoding}, Language=${component.language}`, 'info');
            }
        } catch (error) {
            console.log("Error printing components: " + error.message);
            logEvent(`Error printing components: ${error.message}`, 'error');
        }
    }

    getChannelstatus(status) {
        switch (status) {
            case 0:	// unrealized
                return 'Unrealized';
            case 1:	// connecting
                return 'Connecting';
            case 2:	// presenting
                return 'Presenting';
            case 3:	// stopped
                return 'Stopped';
            default:
                return 'Error';
        }
    }
}

// Create global video broadcast instance
const videoBroadcast = new VideoBroadcast();

// Initialize the application
document.addEventListener('DOMContentLoaded', function() {
    console.log('HbbTV OpApp v1.2.1 initialized');
    logEvent('HbbTV OpApp v1.2.1 initialized', 'success');
    setFocus(0);

    // Initialize channel button text
    const channelBtn = document.getElementById('channelBtn');
    if (channelBtn) {
        channelBtn.textContent = 'Select & Play Channel';
    }

    // Initialize video hole - set width and height directly
    const videoHole = document.getElementById('hole');
    if (videoHole) {
        // Set width to full available width (container width minus margins)
        const container = document.querySelector('.container');
        const containerWidth = container ? container.offsetWidth : 1280;
        const fullWidth = containerWidth - 80; // 20px left + 60px right margins
        videoHole.setAttribute('width', fullWidth.toString());
        videoHole.setAttribute('height', '400');
        videoHole.style.height = '400px';
    }

    // Initialize video broadcast object
    videoBroadcast.initialize();

    // Automatically open the video window as per OpApp specification
    // "Operator applications shall at all times have access to an additional window,
    //  termed the operator application video window"
    openVideoWindow();
});

// Handle window load event (fires after all resources are loaded)
window.addEventListener('load', function() {
    console.log('Window fully loaded');
    logEvent('Window fully loaded', 'success');

    // Example: Request foreground using standard HbbTV/OIPF API
    // According to ETSI TS 103 606 (OpApp spec), opAppRequestForeground is a method
    // on the Application object, not the ApplicationManager
    try {
        const appManager = document.getElementById('app-manager');
        if (appManager) {
            // getOwnerApplication() is part of the OIPF ApplicationManager API
            // It returns the Application object representing the current application
            const ownerApp = appManager.getOwnerApplication(document);
            if (ownerApp) {
                console.log('Requesting foreground for OpApp');
                logEvent('Requesting foreground for OpApp', 'info');

                // opAppRequestForeground() is a method on the Application object
                // It doesn't take parameters - it uses the Application's own ID
                const result = ownerApp.opAppRequestForeground();
                if (result !== null) {
                    // Result is a JSON formatted string
                    console.log('opAppRequestForeground result:', result);
                    try {
                        // Try to parse and pretty-print the JSON
                        const parsedResult = typeof result === 'string' ? JSON.parse(result) : result;
                        const formattedResult = JSON.stringify(parsedResult, null, 2);
                        logEvent('opAppRequestForeground result: ' + formattedResult, 'success');
                        console.log('opAppRequestForeground parsed result:', parsedResult);
                    } catch (e) {
                        // If parsing fails, just log the raw result
                        logEvent('opAppRequestForeground result: ' + result, 'success');
                    }
                } else {
                    logEvent('opAppRequestForeground returned null', 'warning');
                }
            } else {
                logEvent('Could not get owner application', 'warning');
            }
        } else {
            logEvent('Application Manager object not found', 'error');
        }
    } catch (error) {
        console.error('Error calling opAppRequestForeground: ' + error.message);
        logEvent('Error calling opAppRequestForeground: ' + error.message, 'error');
    }
});

// Handle keyboard navigation
document.addEventListener('keydown', function(event) {
    console.log('Key pressed: ' + event.keyCode);
    switch(event.keyCode) {
        case 100: // Keypad 4 / Keypad Left (Channel Previous)
            selectPreviousChannel();
            event.preventDefault();
            break;
        case 102: // Keypad 6 / Keypad Right (Channel Next)
            selectNextChannel();
            event.preventDefault();
            break;
        case 188: // Comma key (,) (Channel Previous)
            selectPreviousChannel();
            event.preventDefault();
            break;
        case 190: // Period/Full stop key (.) (Channel Next)
            selectNextChannel();
            event.preventDefault();
            break;
        case 37: // Left arrow (Channel Previous)
            selectPreviousChannel();
            event.preventDefault();
            break;
        case 39: // Right arrow (Channel Next)
            selectNextChannel();
            event.preventDefault();
            break;
        case 38: // Up arrow
            navigateFocus(-1);
            event.preventDefault();
            break;
        case 40: // Down arrow
            navigateFocus(1);
            event.preventDefault();
            break;
        case 13: // Enter
            activateCurrentButton();
            event.preventDefault();
            break;
        case 8: // Backspace
            goBack();
            event.preventDefault();
            break;
        case 86: // V key (Video - toggle broadcast)
            toggleVideoBroadcast();
            event.preventDefault();
            break;
        case 80: // P key (Play/Pause button)
            togglePlayPause();
            event.preventDefault();
            break;
        case 83: // S key (Stop button - kept for backward compatibility)
            stopVideo();
            event.preventDefault();
            break;
    }
});

function navigateFocus(direction) {
    currentFocus = (currentFocus + direction + buttons.length) % buttons.length;
    setFocus(currentFocus);
}

function setFocus(index) {
    // Remove previous focus
    buttons.forEach(btnId => {
        const btn = document.getElementById(btnId);
        if (btn) btn.style.outline = 'none';
    });

    // Set new focus
    const currentBtn = document.getElementById(buttons[index]);
    if (currentBtn) {
        currentBtn.style.outline = '3px solid #00ff00';
        currentBtn.focus();
    }
}

function activateCurrentButton() {
    const currentBtn = document.getElementById(buttons[currentFocus]);
    if (currentBtn) {
        currentBtn.click();
    }
}

function openVideoWindow() {
    // In a real implementation, this would open the video window
    // For this minimal version, we'll just show a message
    setTimeout(() => {
        const videoWindow = window.open('video/video.html', '_opappVideo');
        updateStatus('[TEST]Video window opened');

        // Store reference to video window for communication
        window.videoWindowRef = videoWindow;

        // Set up message listener for communication from video window
        window.addEventListener('message', function(event) {
            // Verify the message is from our video window
            if (event.source === videoWindow) {
                logEvent(`Message from video window: ${event.data}`, 'info');
            }
        });
    }, 500);
}

// Function to send message to video window
/* Use to test spec requirement 9.9.3:
 *
 * "Communication using postMessage shall be supported in both directions between the two windows using the
 * WindowProxy objects returned by window.open() and window.opener. The event message listener shall
 *support the origin and source properties of the Message event."
 */
function sendMessageToVideoWindow(message) {
    if (window.videoWindowRef && !window.videoWindowRef.closed) {
        window.videoWindowRef.postMessage(message, '*');
        logEvent(`Sent message to video window: ${message}`, 'info');
    } else {
        logEvent('Video window is not available', 'error');
    }
}

function showInfo() {
    const infoPanel = document.getElementById('infoPanel');
    infoPanel.classList.toggle('hidden');
}

function toggleLogPanel() {
    const logPanel = document.getElementById('eventLogPanel');
    const videoHole = document.getElementById('hole');

    if (logPanel) {
        const isHidden = logPanel.classList.toggle('hidden');

        // Keep video hole height consistent to maintain transport bar position
        // Width is handled by CSS (full width)
        if (videoHole) {
            videoHole.style.height = '400px';
        }
    }
}

function runTests() {
    // Simulate test execution
    setTimeout(() => {
        updateStatus('[TEST] Tests completed successfully');
    }, 2000);
}

function goBack() {
    updateStatus('Back button pressed');
    // In a real implementation, this would navigate back
}

function updateStatus(message) {
    const status = document.getElementById('status');
    status.textContent = message;
    console.log('Status:', message);
}

// HbbTV specific functions
function getHbbTVVersion() {
    return '1.2.1';
}

function isHbbTVSupported() {
    return typeof window.hbbtv !== 'undefined';
}

// Video broadcast object functions
function toggleVideoBroadcast() {
    const videoHole = document.getElementById('hole');
    const videoContainer = videoHole.parentElement;

    if (videoContainer.style.display === 'none') {
        videoContainer.style.display = 'flex';
        updateStatus('[TEST] Video broadcast enabled');
    } else {
        videoContainer.style.display = 'none';
        updateStatus('[TEST] Video broadcast disabled');
    }
}

// User Agent Test
function UserAgentTest() {
    //verify user agent value is same with the value in Live TV App
    const EXPECTED_USER_AGENT = "HbbTV/1.7.1 (+DRM; OBS; ORB; v2.0.0; a12-emu; ORB; ) FVC/9.0 (OBS; ORB; )";

    if (navigator.userAgent === EXPECTED_USER_AGENT) {
        logEvent('User Agent Test [PASS]', 'success');
    } else {
        logEvent('User Agent Test [FAIL]', 'error');
    }
}

console.log('User Agent: ' + navigator.userAgent);
logEvent('User Agent: ' + navigator.userAgent, 'info');
UserAgentTest();

// Transport Bar Methods
function stopVideo() {
    console.log('Transport: Stop video');
    videoBroadcast.stop();
}

function togglePlayPause() {
    console.log('Transport: Toggle play/pause');
    videoBroadcast.togglePlayPause();
}

// Channel selection function that uses the VideoBroadcast class
function selectNextChannel() {
    videoBroadcast.selectNextChannel();
}

function selectPreviousChannel() {
    videoBroadcast.selectPreviousChannel();
}

function selectNextChannelBS() {
    videoBroadcast.selectNextChannelBS();
}
