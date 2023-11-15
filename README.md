![Logo](external/logo.png)

# The Open Red Button Project

## Project Overview

The Open Red Button Project (ORB) is a cross-platform solution that extends a range of browsers with HbbTV support. Utilising JavaScript and C++ components, ORB extends browser capabilities and integrates seamlessly with Android's WebView and RDK's WPE. This makes it easier for developers to add HbbTV features to these platforms.

Designed as a reference implementation, ORB facilitates integration but may need modifications, like minor browser patches, for actual product application. Ocean Blue Software (OBS), creators of ORB, specialise in customising ORB for real-world usage. **For commercial support and to discuss how OBS can assist your project, contact OBS at [info@oceanbluesoftware.co.uk](mailto:info@oceanbluesoftware.co.uk).**

## Overview

* **[src/:](src/)** Common polyfill adding HbbTV APIs to the browser (JavaScript).

* **[components/:](components/)** Application Manager and Network Services components, including Media Synchronization and Companion Screen (C++).

* **[android/](android/):** Android library utilising the system WebView and a mock application for integration testing.

* **[rdk/](rdk/):** RDK library utilising the system WPE browser.

See those directories for specific build and integration information.

## Contribute

If you are not otherwise coordinated with the project, raise a [new issue](https://github.com/OpenRedButtonProject/Orb/issues) before starting work on a new fix or feature to avoid duplicated effort.

1. Create or update your fork of the repository to work in.
2. Create a new topic branch on master and complete the fix or feature.
3. Open a pull request. You may find that upstream master has changed and you need to rebase your branch.

The maintainer will squash and merge your branch if it is accepted or they will comment on the pull request.

### Licensing

By contributing to the ORB project, you agree to license your contributions under the Apache License 2.0.

### Resources

* **[ORB C++ Contributors Guide:](https://docs.google.com/document/d/13p6xlcCEkHy__YcaARlAoBQlYd7wqlK4wuEdK13HFds/edit#heading=h.udkzp62bhb46)** Guide for using C++ with ORB.

* **[ORB JS Contributors Guide:](https://docs.google.com/document/d/1OD_ef2eRkz5zv0s7zxmEeH7byhfzPerCL2KPecoTgiQ/edit#heading=h.yfs4lfa8guly)** Guide for using JS with ORB.

## License

The software designated as ORB Software, as indicated in the copyright notice within each file, is licensed under the Apache License, Version 2.0 (the "License"). You can find the full text of the Apache 2.0 License in the [LICENSE](LICENSE) file located in the root directory of this repository.

### Third-Party Code

The ORB project includes third-party code, each subject to its respective open-source license. This encompasses both external dependencies used in our project and code that we have incorporated directly into our repository. The incorporated code, which may be modified, is also used under the terms of its original license. For information about the specific licenses of the third-party code used in the ORB project, refer to our [Third-Party License Documentation](https://docs.google.com/spreadsheets/d/1EuSlycGPBrmEw95TKbG6eeVBz1pkjQXIpYR6IvluAP8/).

