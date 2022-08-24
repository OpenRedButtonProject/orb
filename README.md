![Logo](external/logo.png)

# The Open Red Button Project

The Open Red Button Project is a cross-browser solution for HbbTV.

It is a collection of components that implement the extensions required to support HbbTV applications in a browser:

* [src/](src/) JavaScript polyfill.

* [components/](components/) Application manager and network services native libraries.

### Libraries

For convenience on some systems we package the components as libraries to help with the integration:

* [android/](android/) Android library and test shell that uses the system WebView.

* [rdk/](rdk/) RDK library.

See those directories for specific build and integration information.

## Resources

[ORB Contributors JavaScript Guide](https://docs.google.com/document/d/1s3sOCIP6_o7o9Uk1UzWZLO7NYSQ3uRqeytzSgjPZiDE/)

## How to contribute

If you are not otherwise coordinated with the project, raise an [Issue](https://github.com/OpenRedButtonProject/Orb/issues) before starting work on a new fix or feature to avoid duplicated effort.

1. Create or update your fork of the repository to work in.
2. Create a new topic branch on master and complete the fix or feature.
3. Open a pull request. You may find upstream master has changed and you need to rebase your branch.

The maintainer will squash and merge your branch if it is accepted or they will comment on the pull request.

Contributions to ORB Software must be licensed under the ORB License that can be found in the [LICENSE](LICENSE) file at the top level of this repository.

## License
Software identified as ORB Software in the copyright message in each file is licensed under the ORB License that can be found in the [LICENSE](LICENSE) file at the top level of this repository.

Third-party software is licensed under its respective license, see: [ORB Software Licenses Used](https://docs.google.com/spreadsheets/d/1EuSlycGPBrmEw95TKbG6eeVBz1pkjQXIpYR6IvluAP8/).

