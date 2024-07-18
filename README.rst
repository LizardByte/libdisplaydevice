Overview
========

.. image:: https://img.shields.io/github/actions/workflow/status/lizardbyte/libdisplaydevice/ci.yml.svg?branch=master&label=CI%20build&logo=github&style=for-the-badge
   :alt: GitHub Workflow Status (CI)
   :target: https://github.com/LizardByte/libdisplaydevice/actions/workflows/ci.yml?query=branch%3Amaster

.. image:: https://img.shields.io/codecov/c/gh/LizardByte/libdisplaydevice?token=goyvmDl6J5&style=for-the-badge&logo=codecov&label=codecov
   :alt: Codecov
   :target: https://codecov.io/gh/LizardByte/libdisplaydevice

.. image:: https://img.shields.io/github/stars/lizardbyte/libdisplaydevice.svg?logo=github&style=for-the-badge
   :alt: GitHub stars
   :target: https://github.com/LizardByte/libdisplaydevice

About
-----
libdisplaydevice is a WIP library that provides a common interface for interacting with display devices.
It is intended to be used by applications that need to interact with displays, such as screen capture software,
remote desktop software, and video players.

Initial support is planned for Windows, but could be expanded to other platforms in the future.

Build
-----

Clone
^^^^^

Ensure `git <https://git-scm.com/>`__ is installed and run the following:

.. code-block:: bash

   git clone https://github.com/lizardbyte/libdisplaydevice.git --recurse-submodules
   cd libdisplaydevice && mkdir build && cd build

Windows
^^^^^^^

Requirements
~~~~~~~~~~~~

First you need to install `MSYS2 <https://www.msys2.org>`__, then startup "MSYS2 UCRT64" and execute the following
codes.

Update all packages:
   .. code-block:: bash

      pacman -Suy

Install dependencies:
   .. code-block:: bash

      pacman -S \
        mingw-w64-ucrt-x86_64-binutils \
        mingw-w64-ucrt-x86_64-cmake \
        mingw-w64-ucrt-x86_64-ninja \
        mingw-w64-ucrt-x86_64-toolchain \
        mingw-w64-ucrt-x86_64-boost \
        mingw-w64-ucrt-x86_64-nlohmann-json

Build
~~~~~

.. attention:: Ensure you are in the build directory created during the clone step earlier before continuing.

.. code-block:: bash

   cmake -DCMAKE_BUILD_TYPE:STRING=Debug -G Ninja ..
   ninja

Test
~~~~

.. code-block:: bash

   tests\test_libdisplaydevice.exe


Support
-------

Our support methods are listed in our
`LizardByte Docs <https://lizardbyte.readthedocs.io/en/latest/about/support.html>`__.
