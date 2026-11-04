# gdb script loaded by `make gdb` after connecting to gdbserver on the device.
# Logs every life change, then lets the app run.

set pagination off
set confirm off
set print pretty on

break game_add
commands
  silent
  printf "[life] P%d %+d (was %d)\n", player + 1, delta, game->players[player].life
  continue
end

echo \n[lc] Connected. Life changes print as they happen. Ctrl+C to pause, `quit` to exit.\n\n
continue
