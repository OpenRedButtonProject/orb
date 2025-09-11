// HbbTV OpApp v1.2.1 Minimal Implementation
let currentFocus = 0;
const buttons = ['videoBtn', 'broadcastBtn', 'sendMsgBtn', 'infoBtn'];

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

function clearEventLog() {
    const eventLogContainer = document.getElementById('eventLogContainer');
    if (eventLogContainer) {
        eventLogContainer.innerHTML = '<div class="event-log-entry">Log cleared...</div>';
        logEvent('Event log cleared', 'info');
    }
}

// Video Broadcast Class
class VideoBroadcast {
    constructor() {
        this.videoBroadcast = null;
        this.channelList = [];
        this.currentChannelIndex = 0;
        this.isPlaying = false;
        this.isInitialized = false;
    }

    initialize() {
        try {
            this.videoBroadcast = document.getElementById('hole');
            if (this.videoBroadcast) {
                this.isInitialized = true;
                this.loadChannelList();
                this.setupEventListeners();
                setTimeout(() => {
                    this.selectNextChannel();
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
                const playState = this.getChannelstatus(event.state);
                console.log('PlayStateChange event: ' + playState);
                logEvent(`PlayStateChange: ${playState}`, 'info');
            });
            //ComponentChanged event
            this.videoBroadcast.addEventListener('ComponentChanged', (event) => {
                console.log('ComponentChanged event: ' + event.componentType);
                logEvent(`ComponentChanged: ${event.componentType}`, 'info');
                this.printComponents();
            });
        }
    }

    selectNextChannel() {
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
            this.videoBroadcast.setChannel(channel);
            logEvent(`Channel set: ${channelName}`, 'success');

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
            return true;

        } catch (error) {
            console.log("Could not stop playback: " + error.message);
            logEvent(`Could not stop playback: ${error.message}`, 'error');
            return false;
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
});

// Handle keyboard navigation
document.addEventListener('keydown', function(event) {
    console.log('Key pressed: ' + event.keyCode);
    switch(event.keyCode) {
        case 37: // Left arrow
            navigateFocus(-1);
            event.preventDefault();
            break;
        case 39: // Right arrow
            navigateFocus(1);
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
        case 82: // R key (Red button)
            openVideoWindow();
            event.preventDefault();
            break;
        case 71: // G key (Green button)
            toggleVideoBroadcast();
            event.preventDefault();
            break;
        case 72: // H key (Channel button)
            selectNextChannel();
            event.preventDefault();
            break;
        case 83: // S key (Stop button)
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
    const userAgent = navigator.userAgent;
    //verify user agent format is valid for HbbTV specification for v1.7.1
    const validHbbTVUserAgent = /HbbTV\/1\.7\.1 \(.*\).*/.test(userAgent);
    if (validHbbTVUserAgent) {
        logEvent('User Agent Test [PASS]', 'success');
    } else {
        logEvent('User Agent Test [FAIL]', 'error');
    }
}

console.log('User Agent: ' + navigator.userAgent);
logEvent('User Agent: ' + navigator.userAgent, 'info');
UserAgentTest();

// Initialize video broadcast when page loads
document.addEventListener('DOMContentLoaded', function() {
    videoBroadcast.initialize();
});

// Transport Bar Methods
function stopVideo() {
    console.log('Transport: Stop video');
    videoBroadcast.stop();
}

// Channel selection function that uses the VideoBroadcast class
function selectNextChannel() {
    videoBroadcast.selectNextChannel();
}
