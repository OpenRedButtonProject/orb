#include "opapp.h"
#include "third_party/orb/logging/include/log.h"
#include "application_manager.h"


const int DEFAULT_COUNT_DOWN_TIMEOUT = 60000;

namespace orb
{
// From TS 103 606 V1.2.1 (2024-03) 10.1.3 Table 17
// Since we use a range based approach, we don't define all keys here.
// But putting them here for reference.
const uint16_t VK_CHANNEL_UP   = 401;
const uint16_t VK_CHANNEL_DOWN = 400;
const uint16_t VK_INFO         = 457;
//             VK_GUIDE        = 458;
//             VK_CHANNELS     = 459;
const uint16_t VK_MENU         = 460;
const uint16_t VK_VOLUME_UP    = 462;
//             VK_VOLUME_DOWN  = 463;
//             VK_MUTE         = 464;
//             VK_SUBTITLE     = 465;
//             VK_AUDIO_TRACK  = 466;
//             VK_AUDIO_DESC   = 467;
const uint16_t VK_EXIT         = 468;

// static
std::string OpApp::opAppStateToString(const E_APP_STATE &state)
{
    switch (state)
    {
    case BaseApp::BACKGROUND_STATE:
        return "background";
    case BaseApp::FOREGROUND_STATE:
        return "foreground";
    case BaseApp::TRANSIENT_STATE:
        return "transient";
    case BaseApp::OVERLAID_TRANSIENT_STATE:
        return "overlaid-transient";
    case BaseApp::OVERLAID_FOREGROUND_STATE:
        return "overlaid-foreground";
    default:
        return "undefined";
    }
}

// static
bool OpApp::IsOperatorApplicationKey(const uint16_t keyCode)
{
    // See TS 103 606 V1.2.1 (2024-03) 10.1.3 Table 17
    // OpApp keys form three ranges: 400-401, 457-460, 462-468
    bool isKeyChannelRange = (keyCode >= VK_CHANNEL_DOWN && keyCode <= VK_CHANNEL_UP);
    bool isKeyInfoRange    = (keyCode >= VK_INFO         && keyCode <= VK_MENU);
    bool isKeyVolumeRange  = (keyCode >= VK_VOLUME_UP    && keyCode <= VK_EXIT);
    
    return isKeyChannelRange || isKeyInfoRange || isKeyVolumeRange;
}

OpApp::OpApp(const std::string &url, ApplicationSessionCallback *sessionCallback)
    : BaseApp(APP_TYPE_OPAPP, url, sessionCallback),
      m_countdown([this]() { SetState(BaseApp::BACKGROUND_STATE); })
{
    init();
}

OpApp::OpApp(ApplicationSessionCallback *sessionCallback)
    : BaseApp(APP_TYPE_OPAPP, sessionCallback),
      m_countdown([this]() { SetState(BaseApp::BACKGROUND_STATE); })
{
    init();
}


void OpApp::init()
{
    m_countdownTimeout = DEFAULT_COUNT_DOWN_TIMEOUT;
    m_state = BaseApp::BACKGROUND_STATE; // ETSI TS 103 606 V1.2.1 (2024-03) page 36
    m_scheme = "opapp"; // FREE-273 Temporary scheme for OpApp
}

int OpApp::Load() {
    m_sessionCallback->LoadApplication(
        GetId(), GetLoadedUrl().c_str(), [this]() {

            SetState(m_state);
        });

    // At this point the application is not visible so SetState doesn't work.
    return GetId();
}

bool OpApp::SetState(const E_APP_STATE &state)
{
    std::string current = opAppStateToString(m_state);
    std::string next    = opAppStateToString(state);

    if (!CanTransitionToState(state))
    {
        LOG(INFO) << "Invalid state transition: " << current << " -> " << next;
        return false;
    }

    // FREE-275: Reinstate this check once we have a proper state machine.
    // if (state != m_state) {
    int id = GetId();
    LOG(INFO) << "AppId " << id << "; state transition: " << current << " -> " << next;

    m_sessionCallback->DispatchOperatorApplicationStateChange(id, current, next);

    if (state == BACKGROUND_STATE)
    {
        m_sessionCallback->HideApplication(id);
    }
    else if (state == FOREGROUND_STATE)
    {
        m_sessionCallback->ShowApplication(id);
    }
    else if (state == TRANSIENT_STATE || state == OVERLAID_TRANSIENT_STATE) {
        m_countdown.start(std::chrono::milliseconds(m_countdownTimeout));
    }
    else {
        m_countdown.stop();
    }

    m_state = state;
    return true;
}

bool OpApp::CanTransitionToState(const E_APP_STATE &state)
{
    // FREE-278 - TODO Integrate this check into the state machine above.
    if (state == m_state) {
        return true;
    }

    switch (m_state)
    {
    case BACKGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.1 Page 36
        // Background state can transition to any other state
        return true;

    case FOREGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2 Page 38
        // Allowed transitions from FOREGROUND_STATE
        if (state == BACKGROUND_STATE || state == TRANSIENT_STATE) {
            return true;
        }
        break;

    case TRANSIENT_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.4 Page 41
    case OVERLAID_TRANSIENT_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.6 Page 42
    case OVERLAID_FOREGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.5 Page 41
        // Allowed transitions from these states
        if (state == FOREGROUND_STATE || state == BACKGROUND_STATE) {
            return true;
        }
        break;

    default:
        break;
    }

    return false;
}
} // namespace orb
