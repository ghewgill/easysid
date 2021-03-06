import os
import sys
import tarfile

env = Environment()

if sys.platform == "win32":
    if not os.path.exists("external/resid-0.16/sid.patched.cc"):
        tarfile.open("external/resid-0.16.tar.gz").extractall("external")
        with open("external/resid-0.16/sid.cc") as inf, open("external/resid-0.16/sid.patched.cc", "w") as outf:
            for s in inf:
                s = s.replace("log(2)", "log(2.0)")
                outf.write(s)
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
    env.Command("external/resid-0.16/Makefile", "external/resid-0.16/configure", "cd external/resid-0.16 && env CXXFLAGS=-fPIC ./configure")
    libresid = env.Command("external/resid-0.16/.libs/libresid.a", "external/resid-0.16/Makefile", "cd external/resid-0.16 && make")

env.Append(LIBS=[])
if sys.platform == "win32":
    env.Append(LIBS=["winmm"])
    pcm = ["pcm_win32.cpp"]
elif sys.platform == "darwin":
    env.Append(FRAMEWORKS=["AudioToolbox"])
    pcm = ["pcm_osx.cpp"]
elif os.name == "posix":
    pcm = ["pcm_unix.cpp"]

pcm.extend([
    "pcm_wav.cpp",
])

env.Append(CPPPATH=["external/resid-0.16"])

env.Depends("easysid.cpp", libresid)
libname = "libeasysid" if sys.platform == "win32" else "easysid"
env.SharedLibrary(libname, ["easysid.cpp"] + pcm, LIBS=[libresid] + env["LIBS"])
