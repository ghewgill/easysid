import os
import sys
import tarfile

env = Environment()

env.Command("external/resid-0.16/configure", "external/resid-0.16.tar.gz", lambda target, source, env: tarfile.open(source[0].path).extractall("external"))
env.Command("external/resid-0.16/Makefile", "external/resid-0.16/configure", "cd external/resid-0.16 && ./configure")
libresid = env.Command("external/resid-0.16/.libs/libresid.a", "external/resid-0.16/Makefile", "cd external/resid-0.16 && make")

if sys.platform == "win32":
    pcm = ["pcm_win32.cpp"]
elif sys.platform == "darwin":
    env.Append(FRAMEWORKS=["AudioToolbox"])
    pcm = ["pcm_osx.cpp"]
elif os.name == "posix":
    pcm = ["pcm_unix.cpp"]

env.Append(CPPPATH=["external/resid-0.16"])

env.Depends("easysid.cpp", libresid)
env.SharedLibrary("easysid", ["easysid.cpp"] + pcm, LIBS=[libresid])
