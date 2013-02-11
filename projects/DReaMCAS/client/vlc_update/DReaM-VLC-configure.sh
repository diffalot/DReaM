#!/bin/sh
FFMPEG_DIR=/usr/local/ffmpeg-20050624
WX_DIR=/usr/local/wxGTK-2.6.3/
LIVE_DIR=/usr/local/live
CADEC_DIR=/home/gautam/build
CACRYPTO_DIR=/home/gautam/build
RIGHTSXFACE_DIR=/home/gautam/build

./configure --x-libraries=/usr/X11R6/lib --enable-debug --enable-a52 --enable-ffmpeg --with-ffmpeg-tree=${FFMPEG_DIR} --enable-livedotcom --enable-dvbpsi --with-livedotcom-tree=/home/gautam/work/vlc/live --enable-wx --with-wx-config-path=${WX_DIR} --enable-cas --with-cacrypto=${CACRYPTO_DIR} --with-cadec=${CADEC_DIR} --enable-rightsxface --with-rightsxface=${RIGHTSXFACE_DIR}
