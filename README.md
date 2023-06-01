# sequrunner

## \* This is **work in progress project**, assume that **it just doesn't work at the moment**. \*

---

## Building

1. Clone the repository

- ```
  git clone https://github.com/PanForPancakes/sequrunner.git
  ```

2. Update dependency submodules

- ```
  git submodule git submodule update --init --recursive
  ```

3. Download and unpack ffmpeg prebuild
- Create `external/ffmpeg` folder
- Download system-matching **shared** prebuild from :octocat: [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds/releases/tag/latest)
- Unpack files into `external/ffmpeg` directory
- At this point you should have something like this:
  ```
  external/ffmpeg/
  ├── bin/
  ├── doc/
  ├── include/
  ├── lib/
  └── LICENSE.txt
  ```

4. What next?
- This guide assumes you use CMake-compliant IDE that would help you setup this project further or that you know what you are doing with CMake.  
  ...And so this guide doesn't cover basic CMake usage stuff.
- Most non-intuitive steps were covered at this point.