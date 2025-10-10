#include "base_app.h"


namespace orb
{
int BaseApp::g_id = 0;
const int BaseApp::INVALID_APP_ID = 0;

std::ostream& operator<<(std::ostream& os, const ApplicationType& type) {
    switch (type) {
        case ApplicationType::APP_TYPE_HBBTV:
        os << "HBBTV";
        break;
        case ApplicationType::APP_TYPE_OPAPP:
        os << "OPAPP";
        break;
        default:
        os << "INVALID";
        break;
    }
    return os;
}


BaseApp::BaseApp(const ApplicationType type, const std::string &url, ApplicationSessionCallback *sessionCallback)
    : m_sessionCallback(sessionCallback)
    , m_type(type)
    , m_id(++g_id)
    , m_loadedUrl(url)
{}

BaseApp::BaseApp(ApplicationType type, ApplicationSessionCallback *sessionCallback)
    : m_sessionCallback(sessionCallback)
    , m_type(type)
    , m_id(++g_id)
    , m_loadedUrl("")
{}

ApplicationType BaseApp::GetType() const
{
    return m_type;
}

int BaseApp::GetId() const
{
    return m_id;
}

BaseApp::E_APP_STATE BaseApp::GetState() const
{
    return m_state;
}

std::string BaseApp::GetScheme() const
{
    return m_scheme;
}

std::string BaseApp::GetLoadedUrl() const
{
    return m_loadedUrl;
}

void BaseApp::SetLoadedUrl(const std::string& url)
{
    m_loadedUrl = url;
}

} // namespace orb
