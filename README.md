<div align="center">

<img height="128" src="https://github.com/vslavik/winsparkle/raw/refs/heads/master/artwork/WinSparkle.svg" alt="WinSparkle"/>

# WinSparkle

Automatic updates for Windows applications
<br>
[**Documentation**](https://winsparkle.org/docs/) | [**Support**](https://winsparkle.org/support/)

[![Build WinSparkle](https://github.com/vslavik/winsparkle/actions/workflows/build.yml/badge.svg)](https://github.com/vslavik/winsparkle/actions/workflows/build.yml)
[![Crowdin](https://badges.crowdin.net/winsparkle/localized.svg)](https://crowdin.com/project/winsparkle)

</div>

WinSparkle is heavily inspired by the Sparkle framework for Mac
written by Andy Matuschak and others, to the point of sharing the same
update format (appcasts) and having a very similar user interface.

See https://winsparkle.org for more information about WinSparkle.


 Using prebuilt binaries
-------------------------

The easiest way to use WinSparkle is to either download the prebuilt `WinSparkle.dll`
binary from the releases or use the `WinSparkle` [NuGet package](https://www.nuget.org/packages/WinSparkle/).
Prebuilt binaries are available for x86, x64 and arm64 platforms.


 Bindings
----------

WinSparkle has a C API that makes it easy to use from many modern languages in addition to C/C++. In addition to that, several bindings for popular languages exist:

* [How to use with C#/.NET](https://github.com/vslavik/winsparkle/wiki/Basic-Setup#managed-code--net--c-applications)
* [Python](https://pypi.org/project/pywinsparkle/)
* [Go](https://github.com/abemedia/go-winsparkle)
* [Pascal](https://github.com/vslavik/winsparkle/tree/master/pascal) binding bundled with WinSparkle


 Building from sources
-----------------------

If you prefer to build WinSparkle yourself, you can do so.  You'll have to
compile from a git checkout; some of the dependencies are included as git
submodules.

Check the sources out and initialize the submodules:

    $ git clone https://github.com/vslavik/winsparkle.git
    $ cd winsparkle
    $ git submodule init
    $ git submodule update

To compile the library, open the `WinSparkle.sln` solution in Visual Studio and build it.

There are also unsupported CMake build files in the cmake directory.

 Signing updates
----------------

Updates must be cryptographically signed to prevent tampering. WinSparkle uses the same mechanism for signing and signature verification
as the [Sparkle Project](https://sparkle-project.org/documentation/#dsa-signatures) does. Its tools and verification methods are fully compatible.

Signatures use the EdDSA algorithm with the Ed25519 curve. The public key is included in the app, and enclosures in the appcast must have a signature attached to them.

Older DSA-based signatures are also supported, but they are deprecated and will be removed in a future version. [Upgrade your app to EdDSA](https://github.com/vslavik/winsparkle/wiki/Upgrading-from-DSA-to-EdDSA-signatures) if you still use DSA.

### Companion tool

WinSparkle provides a companion tool, `winsparkle-tool`, to generate keys and sign your updates using EdDSA signatures. This tool is included in the binary package under the `bin` directory, in the NuGet package (in the `tools` directory, pointed to by the `$(WinSparkleTool)` property), or you can compile it from source.

See the output of `winsparkle-tool --help` for more information.

### Prepare signing with EdDSA signatures:

1. First, make yourself a pair of EdDSA keys, using `winsparkle-tool generate-key`. This needs to be done only once.
2. Back up your private key (eddsa_priv.pem) and keep it safe. You don’t want anyone else getting it, and if you lose it, you will not be able to issue any new updates!
3. Add your public key to your project either as a Windows resource or by calling `win_sparkle_set_eddsa_public_key()`

For example:
```
$ winsparkle-tool generate-key --file private.key
Private key saved to private.key
Public key: pXAx0wfi8kGbeQln11+V4R3tCepSuLXeo7LkOeudc/U=

Add the public key to the resource file like this:

    EdDSAPub EDDSA {"pXAx0wfi8kGbeQln11+V4R3tCepSuLXeo7LkOeudc/U="}

or use the API to set it:

    win_sparkle_set_eddsa_public_key("pXAx0wfi8kGbeQln11+V4R3tCepSuLXeo7LkOeudc/U=");
```

### Sign your update

When your update is ready (e.g. `Updater.exe`), sign it and include the signature
in your appcast file:

 - Sign: `winsparkle-tool sign -f private.key Updater.exe`
 - Add the standard output of the previous command as the `sparkle:edSignature`
 attribute of the `enclosure` node in your appcast file.

For example:
```
$ winsparkle-tool sign --verbose --private-key-file private.key Updater.exe
sparkle:edSignature="JhQ69mgRxjNxS35zmMu6bMd9UlkCC/tkCiSR4SXQOfBwwH1FkqYSgNyT5dbWjnw5F1c/6/LqbCGw+WckvJiOBw==" length="1736832"
```


### Legacy DSA signatures

If you still use DSA signatures, you can sign your updates using the `bin/legacy_*.bat` scripts and [the old instructions](https://github.com/vslavik/winsparkle/tree/v0.8.3?tab=readme-ov-file#dsa-signatures), as part of [transitioning to EdDSA signatures](https://github.com/vslavik/winsparkle/wiki/Upgrading-from-DSA-to-EdDSA-signatures).


 Where can I get some examples?
--------------------------------

Download the sources archive and have a look at the
[examples/](https://github.com/vslavik/winsparkle/tree/master/examples) folder.
