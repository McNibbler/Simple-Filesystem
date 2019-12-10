# nufs

An exceptionally jank but sort-of usable EXT-1 style filesystem for Linux. Requires FUSE dev tools
to run. Does not work on WSL-1 due to FUSE dependencies. Native Linux or WSL-2 is required, and
libfuse-dev package is required for compilation and execution. Currently the filesystem mounts a
1MB filesystem with a max filesize of about 120kB. I could make this bigger at some point with
proper indirect pointers but we'll see if that ever actually happens lol. The folder old-attempt is
my first try at making the allocator. It is much much longer and probably more proper with less
hacks, but unfortunatyely I never got it to really work, so whatever. Have fun with this, I suppose.

NOTE: I'm not responsible if you're actually crazy enough to use this to format your drive and you
lose your data.

