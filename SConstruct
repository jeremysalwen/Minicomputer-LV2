print" "
print"Minicomputer-------------- "
print"-                     1/2:configuring"

if ARGUMENTS.get('64bit', 0):
	env = Environment(CCFLAGS = '-m64')
	guienv = Environment(CPPFLAGS = '-m64')
else:
	env = Environment(CCFLAGS = '')
	guienv = Environment(CPPFLAGS = '')

if ARGUMENTS.get('k8', 0):
	env.Append(CCFLAGS = ['-march=k8','-mtune=k8','-m3dnow'])
	guienv.Append(CPPFLAGS = ['-march=k8','-mtune=k8'])
if ARGUMENTS.get('k7', 0):
	env.Append(CCFLAGS = ['-march=athlon-xp','-mtune=athlon-xp'])
	guienv.Append(CPPFLAGS = ['-march=athlon-xp','-mtune=athlon-xp'])

# env.Append(CCFLAGS = '  -O3 -mfpmath=sse -msse -msse2  -fverbose-asm  -ffast-math -funit-at-a-time -fpeel-loops -ftracer -funswitch-loops -Wall -fmessage-length=0')

env.Append(CCFLAGS = ['-O3','-msse','-fwhole-program','-fverbose-asm','-ftree-vectorize','-ffast-math', '-funit-at-a-time', '-fpeel-loops', '-ftracer','-funswitch-loops', '-mfpmath=sse'])
conf = Configure(env)

if not conf.CheckLibWithHeader('jack', 'jack/jack.h','c'):
	print 'Did not find jack, exiting!'
	Exit(1)
if not conf.CheckLibWithHeader('lo', 'lo/lo.h','c'):
	print 'Did not find liblo for OSC, exiting!'
	Exit(1)
if not conf.CheckLibWithHeader('asound', 'alsa/asoundlib.h','c'):
	print 'Did not find alsa, exiting!'
	Exit(1)
if not conf.CheckLibWithHeader('pthread', 'pthread.h','c'):
	print 'Did not find pthread library, exiting!'
	Exit(1)
env = conf.Finish()

print"-                    checking dependencies for the editor:"

guiconf = Configure(guienv)
if not guiconf.CheckLibWithHeader('lo', 'lo/lo.h','c'):
	print 'Did not find liblo for OSC, exiting!'
	Exit(1)
if not guiconf.CheckLibWithHeader('fltk', 'FL/Fl.H','c++'):
	print 'Did not find FLTK for the gui, exiting!'
	Exit(1)
if not guiconf.CheckLibWithHeader('asound', 'alsa/asoundlib.h','c'):
	print 'Did not find alsa, exiting!'
	Exit(1)
if not guiconf.CheckLibWithHeader('pthread', 'pthread.h','c'):
	print 'Did not find pthread library, exiting!'
	Exit(1)
guienv = guiconf.Finish()
guienv.Append(CPPFLAGS = ['-O3','-Wall','-fmessage-length=0'])

print"-                     2/2:compiling"
print"-                     building the engine:"

env.Program('minicomputer','cpu/main.c');

print""
print"-                     building the editor:"

guienv.Program('miniEditor',['editor/main.cpp','editor/Memory.cpp','editor/syntheditor.cxx']);


