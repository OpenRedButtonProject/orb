#include "base_app.h"
#include "third_party/orb/logging/include/log.h"


namespace orb
{
int BaseApp::g_id = 0;
const int BaseApp::INVALID_APP_ID = 0;

const uint16_t VK_RED = 403;
const uint16_t VK_GREEN = 404;
const uint16_t VK_YELLOW = 405;
const uint16_t VK_BLUE = 406;
const uint16_t VK_UP = 38;
const uint16_t VK_DOWN = 40;
const uint16_t VK_LEFT = 37;
const uint16_t VK_RIGHT = 39;
const uint16_t VK_ENTER = 13;
const uint16_t VK_BACK = 461;
const uint16_t VK_PLAY = 415;
const uint16_t VK_STOP = 413;
const uint16_t VK_PAUSE = 19;
const uint16_t VK_FAST_FWD = 417;
const uint16_t VK_REWIND = 412;
const uint16_t VK_NEXT = 425;
const uint16_t VK_PREV = 424;
const uint16_t VK_PLAY_PAUSE = 402;
// const uint16_t VK_RECORD = 416;
const uint16_t VK_PAGE_UP = 33;
const uint16_t VK_PAGE_DOWN = 34;
const uint16_t VK_INFO = 457;
const uint16_t VK_NUMERIC_START = 48;
const uint16_t VK_NUMERIC_END = 57;
const uint16_t VK_ALPHA_START = 65;
const uint16_t VK_ALPHA_END = 90;


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

// Needs testing!
uint16_t BaseApp::SetKeySetMask(const uint16_t keySetMask, const std::vector<uint16_t> &otherKeys) {

    m_keySetMask = keySetMask;
    if ((keySetMask & KEY_SET_OTHER) == KEY_SET_OTHER) {
        m_otherKeys = otherKeys; // Survived all checks
    }

    return keySetMask;
}

bool BaseApp::InKeySet(const uint16_t keyCode)
{
    // Check regular key set mask first.
    if ((m_keySetMask & GetKeySetMaskForKeyCode(keyCode)) != 0)
    {
        return true;
    }

    // Check other keys if the key set mask contains KEY_SET_OTHER.
    if ((m_keySetMask & KEY_SET_OTHER) == KEY_SET_OTHER) {
        auto it = std::find(m_otherKeys.begin(), m_otherKeys.end(), keyCode);
        if (it != m_otherKeys.end()) {
            return true;
        }
    }

    return false;
}

// static
uint16_t BaseApp::GetKeySetMaskForKeyCode(const uint16_t keyCode)
{
    if (IsKeyNavigation(keyCode))
    {
        return KEY_SET_NAVIGATION;
    }
    else if (IsKeyNumeric(keyCode))
    {
        return KEY_SET_NUMERIC;
    }
    else if (IsKeyAlpha(keyCode))
    {
        return KEY_SET_ALPHA;
    }
    else if (IsKeyVcr(keyCode))
    {
        return KEY_SET_VCR;
    }
    else if (IsKeyScroll(keyCode))
    {
        return KEY_SET_SCROLL;
    }
    else if (keyCode == VK_RED)
    {
        return KEY_SET_RED;
    }
    else if (keyCode == VK_GREEN)
    {
        return KEY_SET_GREEN;
    }
    else if (keyCode == VK_YELLOW)
    {
        return KEY_SET_YELLOW;
    }
    else if (keyCode == VK_BLUE)
    {
        return KEY_SET_BLUE;
    }
    else if (keyCode == VK_INFO)
    {
        return KEY_SET_INFO;
    }
    // else if (keyCode == VK_RECORD)
    // {
    //     return KEY_SET_OTHER;
    // }

    return 0;
}

// static
bool BaseApp::IsKeyNavigation(const uint16_t code)
{
    return code == VK_UP ||
           code == VK_DOWN ||
           code == VK_LEFT ||
           code == VK_RIGHT ||
           code == VK_ENTER ||
           code == VK_BACK;
}

// static
bool BaseApp::IsKeyNumeric(const uint16_t code)
{
    return code >= VK_NUMERIC_START && code <= VK_NUMERIC_END;
}

// static
bool BaseApp::IsKeyAlpha(const uint16_t code)
{
    return code >= VK_ALPHA_START && code <= VK_ALPHA_END;
}

// static
bool BaseApp::IsKeyVcr(const uint16_t code)
{
    return code == VK_PLAY ||
           code == VK_STOP ||
           code == VK_PAUSE ||
           code == VK_FAST_FWD ||
           code == VK_REWIND ||
           code == VK_NEXT ||
           code == VK_PREV ||
           code == VK_PLAY_PAUSE;
}

// static
bool BaseApp::IsKeyScroll(const uint16_t code)
{
    return code == VK_PAGE_UP ||
           code == VK_PAGE_DOWN;
}

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



} // namespace orb
