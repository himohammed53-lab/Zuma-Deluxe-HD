#!/bin/sh
# Fuse sizzle for the bomb in the frog's mouth (made from noise, ~1.3 s).
# Project Owner & Creator: Muhamed Alzarqi / AI Programming Assistant: Claude (Anthropic)
ffmpeg -y -hide_banner -loglevel error \
  -f lavfi -i "anoisesrc=d=1.3:c=white:a=0.6:r=44100" \
  -f lavfi -i "anoisesrc=d=1.3:c=pink:a=0.9:r=44100" \
  -filter_complex "[0]highpass=f=2500,lowpass=f=9000,volume=0.55[hiss];\
[1]highpass=f=900,lowpass=f=4000,agate=threshold=0.35:ratio=9:attack=0.5:release=8,volume=1.6[crack];\
[hiss][crack]amix=inputs=2:normalize=0,afade=t=in:d=0.05,afade=t=out:st=1.15:d=0.15,volume=0.8" \
  -ac 1 -c:a libvorbis -q:a 5 "$(dirname "$0")/../content/sounds/fuse.ogg"
