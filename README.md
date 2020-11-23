
# Spaghettis

Yet another fork of [Pure Data](http://msp.ucsd.edu/) (in development, alpha).

## Rationale

```
The Pure Data source code has been utterly refactored.
For me it was a requirement before to think about any improvements.
During all this years the goal was to make it more simple (understandable) and consistent.
For that, features has been removed and others added.
The DSP now runs in its own obstruction-free thread. It is supposed to never been blocked.
Next big step is to use JUCE for the GUI.
The idea is to be able later to make plugins easiliy from your patches.
Platforms such Raspberry Pi will be strongly focused (e.g. to hack your own effects unit).
Bind it to a script language (Python or Lua) will make it more powerful at last.
```

## Platforms

Tested on Ubuntu 20.04, macOS Big Sur (Intel only), Manjaro and Raspbian Buster (RPI4).
        
## Dependencies

On Debian-based following packages are required:
    
    - tk8.6
    - tcl8.6
    - libasound2-dev
    - libjack-jackd2-dev
    
On Arch-based following packages are required:
    
    - tk
    - jack

## Run

- Open a terminal and run the `Spaghettis/build.sh` script.
- Nothing will be installed in your system.
- On GNU/Linux execute `Spaghettis/build/bin/spaghettis`.

## Alternate

- On Arch-based you can use the [AUR](https://aur.archlinux.org/packages/spaghettis-git/) from  [Milkii Brewster](https://github.com/mxmilkiib).

## Comments

- Spaghettis uses new filename extensions ***pdpatch*** and ***pdhelp***.
- You can open the [tutorials](https://github.com/Spaghettis/Tools) for a quick look.
- Else, copy and rename your Pure Data patches (cross your fingers for compatibility).

## Residuals

**macOS**

    ~/Library/Application Support/Spaghettis/
    ~/Library/Preferences/org.puredata.spaghettis.plist

**Linux**

    ~/.config/spaghettis/

## Credits

- Pure Data by [Miller Puckette & others](http://msp.ucsd.edu/Software/pd-README.txt)
- Belle by [William Andrew Burnson](https://github.com/burnson)
- TinyExpr by [Lewis Van Winkle](https://github.com/codeplea/tinyexpr)
- Mersenne Twister by [T. Nishimura & M. Matsumoto](http://www.math.sci.hiroshima-u.ac.jp/~m-mat)
- FFT by [Takuya Ooura](http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html)
    
#### Licensed mostly under the [BSD-3-Clause license](https://opensource.org/licenses/BSD-3-Clause).
