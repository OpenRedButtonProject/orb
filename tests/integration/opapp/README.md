# OpApp Test Material

## Minimal HbbTV OpApp v1.2.1

A simple, minimal HbbTV Operator Application (OpApp) implementation based on the HbbTV v1.2.1 specification.

AI Generate README!!

### Overview

This minimal OpApp demonstrates the core functionality required for HbbTV compliance:

- **Main Application** (`index.html`): Primary OpApp interface with navigation
- **Video Broadcast Hole**: Direct cut-through to underlying video content
- **Remote Control Support**: Full keyboard and remote control navigation
- **HbbTV Compliance**: Proper DOCTYPE and namespace declarations

### Features

#### Core Functionality
- ✅ HbbTV v1.2.1 DOCTYPE compliance
- ✅ XML namespace declarations
- ✅ Remote control navigation
- ✅ Video broadcast hole support
- ✅ Keyboard navigation
- ✅ Focus management
- ✅ Status reporting

#### Navigation Support
- **Arrow Keys**: Navigate between interactive elements
- **Enter/OK**: Activate selected element
- **Back**: Return to previous screen
- **Red Button**: Open video window
- **Green Button**: Toggle video broadcast
- **Space**: Play/pause video

#### Video Broadcast Features
- Direct video cut-through using `<object type="video/broadcast">`
- Transparent UI overlay
- Toggle broadcast visibility
- Clean positioning without container interference

### HbbTV Compliance

#### DOCTYPE Declaration
```html
<!DOCTYPE html PUBLIC "-//HbbTV//1.2.1//EN" "http://www.hbbtv.org/dtd/HbbTV-1.2.1.dtd">
```

#### XML Namespace
```html
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en" class="hbbtv-viewport">
```

#### Key Features
- Proper HbbTV viewport class
- XML namespace support
- Remote control event handling
- Video broadcast integration
- Status reporting

### Development Notes

#### Browser Testing
- Works in modern browsers for development
- HbbTV API detection for compatibility
- Fallback to test mode when API unavailable

#### Remote Control Mapping
- Arrow keys: Navigation
- Enter: Activation
- Backspace: Back
- R key: Red button (video window)
- G key: Green button (toggle broadcast)
- Space: Play/pause

#### Video Broadcast
- Uses `<object type="video/broadcast">` for direct cut-through
- Positioned directly without container interference
- Transparent UI overlay
- Toggle functionality

### Testing Checklist

- [ ] Main application loads
- [ ] Navigation works with arrow keys
- [ ] Enter key activates buttons
- [ ] Video broadcast hole works
- [ ] Broadcast toggle works
- [ ] Status updates display
- [ ] HbbTV API detection works
- [ ] Remote control simulation works

### Future Enhancements

- Add more video formats support
- Implement EPG integration
- Add subtitle support
- Enhanced error handling
- More sophisticated navigation
- Additional HbbTV API features
