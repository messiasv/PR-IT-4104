# Image segmentation

Program that separates the foreground from the background to display the foreground elements in front of a chosen background.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

What things you need to install the software and how to install them

* OpenGL
* OpenCV

### Installing

* Installing OpenCV
```
sudo apt install build-essential make cmake git libgtk2.0-dev pkg-config python python-dev python-numpy libavcodec-dev libavformat-dev libswscale-dev libjpeg-dev libpng-dev libtiff-dev
cd ~/Downloads
git clone https://github.com/itseez/opencv
mv opencv /opt
cd /opt/opencv
git checkout 2.4.10.1 #or whatever version you want
sudo mkdir build
cd build
sudo cmake -j4 -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local ..
sudo make -j4
sudo make -j4 install
sudo ldconfig
```

## Deployment

```
make
./seg_image
```

## Authors

* **Hafiz Hoxha** - [hafiz94](https://github.com/hafiz94)
* **Jennifer Lim** - [refinnej](https://github.com/refinnej)
* **Valentin Messias** - [messiasv](https://github.com/messiasv)
* **Vincent Jouhaud** - [ashyla](https://github.com/ashyla)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
