#include "opapp.h"
#include "log.h"
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
    : BaseApp(BaseApp::OPAPP_TYPE, url, sessionCallback),
      m_countdown([this]() { SetState(BaseApp::BACKGROUND_STATE); })
{
    init();
}

OpApp::OpApp(ApplicationSessionCallback *sessionCallback)
    : BaseApp(BaseApp::OPAPP_TYPE, sessionCallback),
      m_countdown([this]() { SetState(BaseApp::BACKGROUND_STATE); })
{
    init();
}


void OpApp::init()
{
    m_state = BaseApp::BACKGROUND_STATE; // ETSI TS 103 606 V1.2.1 (2024-03) page 36
    m_scheme = "opapp"; // FREE-273 Temporary scheme for OpApp
}

bool OpApp::SetState(const E_APP_STATE &state)
{
    if (CanTransitionToState(state))
    {
        if (state != m_state)
        {
            std::string previous = opAppStateToString(m_state);
            std::string next = opAppStateToString(state);
            m_state = state;
            m_sessionCallback->DispatchOperatorApplicationStateChange(GetId(), previous, next);

            if (state == BaseApp::BACKGROUND_STATE)
            {
                m_sessionCallback->HideApplication(GetId());
            }
            else
            {
                m_sessionCallback->ShowApplication(GetId());
            }
        }

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
    LOG(LOG_INFO, "Invalid state transition: %d -> %d", m_state, state);
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

        default:
            break;
        }
        return false;
    }
    return true;
}
} // namespace orb
