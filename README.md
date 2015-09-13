## Keyfinder Command Line Interface

This small utility is the automation-oriented DJs best friend. By making use of
[Ibrahim Sha'ath](http://www.ibrahimshaath.co.uk/)'s high quality
[libKeyFinder](https://github.com/ibsh/libKeyFinder/) library, this utility can
be used to estimate the musical key of many different audio file formats.

[![Build Status](https://travis-ci.org/EvanPurkhiser/keyfinder-cli.svg?branch=master)](https://travis-ci.org/EvanPurkhiser/keyfinder-cli)

### Usage and Expected Behavior

Installing this software exposes the `keyfinder-cli` command on your system.

The most basic usage of this utility is to provide a path to an audio file, it
will quickly compute the estimated global music key of the audio file and print
it to stdout.

```sh
$ keyfinder-cli AMajor.mp3
A
```

In the case that there is no key (silence) nothing will be printed to stdout
and the program will exit with a 0 status code.

### Different key notations

Three different key notations are supported and can be toggled:

 1. `-n standard` for **Standard Key Notation**

    Keys are outputed using the standard notation.  For example, `Eb` is
    equivelant to E flat. `A` is equivelant to A Major.  Sharps are not used.
    This is the default notation mode used to output keys when no other otpions
    are specified

 2. `-n openkey` for [**Open Key Notation**](https://www.beatunes.com/en/open-key-notation.html)

     Keys are outputed using BeaTunes Open Key notation. Keys like `1m` are
     equivelant to C Major. This is what Traktor uses for it's key notation.

 3. `-n camelot` for [**Camelot Key Notation**](http://www.mixedinkey.com/HowTo)

    Keys are outputed using the Camelot Easymix Wheel notation.  Similar to
    Open Key notation `8B` is equivelant to C Major.


