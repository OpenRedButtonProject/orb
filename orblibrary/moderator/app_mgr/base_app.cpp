#include "base_app.h"


namespace orb
{
int BaseApp::g_id = 0;
const int BaseApp::INVALID_APP_ID = 0;

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
