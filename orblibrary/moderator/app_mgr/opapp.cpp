#include "opapp.h"
#include "third_party/orb/logging/include/log.h"
#include "application_manager.h"

#define COUNT_DOWN_TIMEOUT 60000

namespace orb
{
static std::string opAppStateToString(const BaseApp::E_APP_STATE &state)
{
    switch (state)
    {
    case BaseApp::BACKGROUND_STATE: return "background";
    case BaseApp::FOREGROUND_STATE: return "foreground";
    case BaseApp::TRANSIENT_STATE: return "transient";
    case BaseApp::OVERLAID_TRANSIENT_STATE: return "overlaid-transient";
    case BaseApp::OVERLAID_FOREGROUND_STATE: return "overlaid-foreground";
    default: break;
    }
    // should never get here
    return "undefined";
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
    if (CanTransitionToState(state))
    {
        // FREE-275: Reinstate this check once we have a proper state machine.
        // if (state != m_state) {
        int id = GetId();
        LOG(INFO) << "AppId " << id << "; state transition: " << m_state << " -> " << state;
        std::string previous = opAppStateToString(m_state);
        std::string next = opAppStateToString(state);
        m_state = state;
        m_sessionCallback->DispatchOperatorApplicationStateChange(id, previous, next);

        if (state == BaseApp::BACKGROUND_STATE)
        {
            m_sessionCallback->HideApplication(id);
        }
        else // Need to support other states
        {
            m_sessionCallback->ShowApplication(id);
        }
        // } // if (state != m_state)

        if (state == BaseApp::TRANSIENT_STATE || state == BaseApp::OVERLAID_TRANSIENT_STATE)
        {
            m_countdown.start(std::chrono::milliseconds(COUNT_DOWN_TIMEOUT));
        }
        else
        {
            m_countdown.stop();
        }
        return true;
    }
    LOG(INFO) << "Invalid state transition: " << GetState() << " -> " << state;
    return false;
}

bool OpApp::CanTransitionToState(const E_APP_STATE &state)
{
    if (state != m_state)
    {
        switch (m_state)
        {
        case FOREGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2 Page 38
            // Allowed transitions from FOREGROUND_STATE
            if (state == BaseApp::BACKGROUND_STATE || state == BaseApp::TRANSIENT_STATE) {
                return true;
            }
            break;

        case TRANSIENT_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.4 Page 41
        case BaseApp::OVERLAID_TRANSIENT_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.6 Page 42
        case BaseApp::OVERLAID_FOREGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.5 Page 41
            // Allowed transitions from these states
            if (state == BaseApp::FOREGROUND_STATE || state == BaseApp::BACKGROUND_STATE) {
                return true;
            }
            break;
        case BACKGROUND_STATE: // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2 Page 38
            // Allowed transitions from BACKGROUND_STATE
            if (state == BaseApp::FOREGROUND_STATE) {
                return true;
            }
            break;
        default:
            break;
        }
        return false;
    }
    return true;
}
} // namespace orb
