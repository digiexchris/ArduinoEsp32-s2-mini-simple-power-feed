/usr/bin/openocd -s /workspaces/local -f /root/.vscode-server/extensions/marus25.cortex-debug-1.12.1/support/openocd-helpers.tcl -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000"