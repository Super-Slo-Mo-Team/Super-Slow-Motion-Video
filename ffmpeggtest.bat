@echoo off
ffmpeg -ss %1 -i %2  -c:v libx264 -crf 18 -to %3 -c:a copy %4