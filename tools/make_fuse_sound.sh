#!/bin/sh
# Fuse sizzle for the bomb in the frog's mouth: a smooth, steady hiss
# with soft crackle (2 s, replayed while the bomb is in the mouth).
# Project Owner & Creator: Muhamed Alzarqi / AI Programming Assistant: Claude (Anthropic)
ffmpeg -y -hide_banner -loglevel error \
  -f lavfi -i "anoisesrc=d=2:c=white:a=0.5:r=44100:s=7" \
  -f lavfi -i "anoisesrc=d=2:c=brown:a=0.8:r=44100:s=3" \
  -filter_complex "[0]highpass=f=3000,lowpass=f=8000,volume=0.35[hiss];\
[1]highpass=f=1500,lowpass=f=3500,volume=0.6[warm];\
[hiss][warm]amix=inputs=2:normalize=0,afade=t=in:d=0.03,afade=t=out:st=1.95:d=0.05,volume=0.6" \
  -ac 1 -c:a libvorbis -q:a 5 "$(dirname "$0")/../content/sounds/fuse.ogg"
