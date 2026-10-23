# NanoSoft DOS 1.0

NanoSoft DOS is a tiny experimental x86 hobby operating system. It boots through GRUB/Multiboot, writes to VGA text memory, and provides a very small keyboard-driven command prompt.

## Commands

- `HELP` — show commands
- `VER` — show version
- `CLS` — clear the screen
- `ECHO hello` — print text
- `ABOUT` — show information
- `REBOOT` — request a virtual-machine reboot

## Build the ISO on Ubuntu/Debian

Install dependencies:

```sh
sudo apt update
sudo apt install build-essential gcc-multilib grub-pc-bin xorriso mtools
```

Then run:

```sh
chmod +x build.sh
./build.sh
```

The output is `NanoSoft-DOS.iso`.

## Build it from an iPhone using GitHub Actions

1. Create a new GitHub repository.
2. Upload all files from this project, keeping their folder paths.
3. Commit the files to the `main` branch.
4. Open the repository's **Actions** tab and choose **Build NanoSoft DOS ISO**.
5. When the workflow finishes successfully, open the run and download the `NanoSoft-DOS-ISO` artifact.
6. Extract the downloaded ZIP to get `NanoSoft-DOS.iso`.

The workflow also runs automatically when you push changes to `main`.

## UTM SE notes

Create a new emulated PC/legacy x86 machine in UTM SE and attach `NanoSoft-DOS.iso` as the boot ISO/CD-ROM. This project targets 32-bit x86 BIOS/Multiboot booting, so choose an x86 machine configuration that supports legacy BIOS boot. UTM SE compatibility and performance depend on the device and chosen emulation settings.

## Limitations

This is a learning prototype, not a replacement for MS-DOS or a general-purpose OS. The shell currently supports a basic US keyboard layout, simple text output, and a few built-in commands. It has no filesystem, programs, networking, sound, or graphical desktop yet.
