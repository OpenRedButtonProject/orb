#include "opapp.h"
#include "log.h"

#define COUNT_DOWN_TIMEOUT 60000

static std::string opAppStateToString(const HbbTVApp::E_APP_STATE &state);

/**
 * Create opapp from url.
 * 
 * @throws std::runtime_error
 */
OpApp::OpApp(const std::string &url, std::shared_ptr<OpApp::SessionCallback> sessionCallback)
    : HbbTVApp(url, sessionCallback),
    m_state(BACKGROUND_STATE) // ETSI TS 103 606 V1.2.1 (2024-03) page 36
{ }

/**
 * Create opapp from Ait description.
 * 
 * @throws std::runtime_error
 */
OpApp::OpApp(const Ait::S_AIT_APP_DESC &desc, bool isNetworkAvailable, std::shared_ptr<OpApp::SessionCallback> sessionCallback)
    : HbbTVApp(
        desc,
        Utils::MakeInvalidDvbTriplet(),
        isNetworkAvailable,
        "",
        true,
        false,
        sessionCallback
    ),
    m_state(BACKGROUND_STATE) // ETSI TS 103 606 V1.2.1 (2024-03) page 36
{ }

/**
 * Create opapp from url and inherit another opapp's state (ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.1).
 * 
 * @throws std::runtime_error
 */
OpApp::OpApp(const OpApp &other, const std::string &url)
    : OpApp(url, other.m_sessionCallback),
    m_state(other.GetState())
{
    if (!other.m_countdown.isStopped())
    {
        m_countdown.start(other.m_countdown.remaining());
    }
}

/**
 * Set the application state.
 * 
 * @param state The desired state to transition to.
 * @returns true if transitioned successfully to the desired state, false otherwise.
 */
bool OpApp::SetState(const E_APP_STATE &state)
{
    if (CanTransitionToState(state))
    {
        if (state != m_state)
        {
            std::string previous = opAppStateToString(m_state);
            std::string next = opAppStateToString(state);
            m_state = state;
            static_cast<SessionCallback*>(m_sessionCallback.get())->DispatchOperatorApplicationStateChange(GetId(), previous, next);
            
            if (state == BACKGROUND_STATE)
            {
                m_sessionCallback->HideApplication(GetId());
            }
            else
            {
                m_sessionCallback->ShowApplication(GetId());
            }
        }
        
        if (state == TRANSIENT_STATE || state == OVERLAID_TRANSIENT_STATE)
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

bool OpApp::TransitionToBroadcastRelated()
{
    // ETSI TS 103 606 V1.2.1 (2024-03) 6.3.3.2 Note 2
    if (m_state == FOREGROUND_STATE)
    {
        return HbbTVApp::TransitionToBroadcastRelated();
    }
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
    return true;
}


static std::string opAppStateToString(const HbbTVApp::E_APP_STATE &state)
{
    switch (state)
    {
        case HbbTVApp::BACKGROUND_STATE: return "background";
        case HbbTVApp::FOREGROUND_STATE: return "foreground";
        case HbbTVApp::TRANSIENT_STATE: return "transient";
        case HbbTVApp::OVERLAID_TRANSIENT_STATE: return "overlaid-transient";
        case HbbTVApp::OVERLAID_FOREGROUND_STATE: return "overlaid-foreground";
        default: break;
    }
    // should never get here
    return "undefined";
}