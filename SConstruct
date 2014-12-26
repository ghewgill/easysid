import os
import sys
import tarfile

env = Environment()

if sys.platform == "win32":
    def extract_resid(target, source, env):
        tarfile.open(source[0].path).extractall("external")
        with open("external/resid-0.16/sid.cc") as inf, open("external/resid-0.16/sid.patched.cc", "w") as outf:
            for s in inf:
                s = s.replace("log(2)", "log(2.0)")
                outf.write(s)
    env.Command("external/resid-0.16/sid.patched.cc", "external/resid-0.16.tar.gz", extract_resid)
    libresid = env.Library("external/resid-0.16/resid.lib", [
        "external/resid-0.16/envelope.cc",
        "external/resid-0.16/extfilt.cc",
        "external/resid-0.16/filter.cc",
        "external/resid-0.16/pot.cc",
        "external/resid-0.16/sid.patched.cc",
        "external/resid-0.16/voice.cc",
        "external/resid-0.16/wave.cc",
        "external/resid-0.16/wave6581_PST.cc",
        "external/resid-0.16/wave6581_PS_.cc",
        "external/resid-0.16/wave6581_P_T.cc",
        "external/resid-0.16/wave6581__ST.cc",
        "external/resid-0.16/wave8580_PST.cc",
        "external/resid-0.16/wave8580_PS_.cc",
        "external/resid-0.16/wave8580_P_T.cc",
        "external/resid-0.16/wave8580__ST.cc",
    ])
else:
    env.Command("external/resid-0.16/configure", "external/resid-0.16.tar.gz", lambda target, source, env: tarfile.open(source[0].path).extractall("external"))
    env.Command("external/resid-0.16/Makefile", "external/resid-0.16/configure", "cd external/resid-0.16 && ./configure")
    libresid = env.Command("external/resid-0.16/.libs/libresid.a", "external/resid-0.16/Makefile", "cd external/resid-0.16 && make")

if sys.platform == "win32":
    env.Append(LIBS=["winmm", libresid])
    pcm = ["pcm_win32.cpp"]
elif sys.platform == "darwin":
    env.Append(FRAMEWORKS=["AudioToolbox"])
    pcm = ["pcm_osx.cpp"]
elif os.name == "posix":
    pcm = ["pcm_unix.cpp"]

env.Append(CPPPATH=["external/resid-0.16"])

env.Depends("easysid.cpp", libresid)
libname = "libeasysid" if sys.platform == "win32" else "easysid"
env.SharedLibrary(libname, ["easysid.cpp"] + pcm)
