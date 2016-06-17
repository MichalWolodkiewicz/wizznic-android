import subprocess
import sys
ndk_result = subprocess.call("ndk-build", shell=True)
if ndk_result != 0:
	sys.exit(-1)
subprocess.call("ant clean debug uninstall install", shell=True)