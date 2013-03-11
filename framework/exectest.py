#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

import os
import subprocess
import shlex
import types

from core import Test, testBinDir, TestResult

#############################################################################
##### Platform global variables
#############################################################################
if 'PIGLIT_PLATFORM' in os.environ:
    PIGLIT_PLATFORM = os.environ['PIGLIT_PLATFORM']
else:
    PIGLIT_PLATFORM = ''

#############################################################################
##### ExecTest: A shared base class for tests that simply run an executable.
#############################################################################

class ExecTest(Test):
	def __init__(self, command):
		Test.__init__(self)
		self.command = command
		self.split_command = os.path.split(self.command[0])[1]
		self.env = {}

		if isinstance(self.command, basestring):
			self.command = shlex.split(str(self.command))

		self.skip_test = self.check_for_skip_scenario(command)

	def interpretResult(self, out, returncode, results):
		raise NotImplementedError
		return out

	def run(self, valgrind):
		fullenv = os.environ.copy()
		for e in self.env:
			fullenv[e] = str(self.env[e])

		if self.command is not None:
			command = self.command

			if valgrind:
				command[:0] = ['valgrind', '--quiet', '--error-exitcode=1', '--tool=memcheck']

			i = 0
			while True:
				if os.environ.has_key("ANDROID_BUILD_TOP") == True and platform.system() == 'Linux':
					writepath = os.environ['HOME']

					self.scriptfile = open(writepath + "/piglit_" +self.command_name+ "_android.sh", "w")
					self.scriptfile.write(self.sh_script_remote)
					self.scriptfile.close()

					self.scriptfile = open(writepath + "/piglit_" +self.command_name+ "_host.sh", "w")
					self.scriptfile.write(self.sh_script_host)
					self.scriptfile.close()

				if self.skip_test:
					out = "PIGLIT: {'result': 'skip'}\n"
					err = ""
					returncode = None
				else:
					(out, err, returncode) = \
						self.get_command_result(command, fullenv)

				if os.environ.has_key("ANDROID_BUILD_TOP") == True and platform.system() == 'Linux':
					os.remove(writepath + "/piglit_" +self.command_name+ "_host.sh")
					os.remove(writepath + "/piglit_" +self.command_name+ "_android.sh")

				# https://bugzilla.gnome.org/show_bug.cgi?id=680214 is
				# affecting many developers.  If we catch it
				# happening, try just re-running the test.
				if out.find("Got spurious window resize") >= 0:
					i = i + 1
					if i >= 5:
						break
				else:
					break

			# proc.communicate() returns 8-bit strings, but we need
			# unicode strings.  In Python 2.x, this is because we
			# will eventually be serializing the strings as JSON,
			# and the JSON library expects unicode.  In Python 3.x,
			# this is because all string operations require
			# unicode.  So translate the strings into unicode,
			# assuming they are using UTF-8 encoding.
			#
			# If the subprocess output wasn't properly UTF-8
			# encoded, we don't want to raise an exception, so
			# translate the strings using 'replace' mode, which
			# replaces erroneous charcters with the Unicode
			# "replacement character" (a white question mark inside
			# a black diamond).
			out = out.decode('utf-8', 'replace')
			err = err.decode('utf-8', 'replace')

			results = TestResult()

			if self.skip_test:
				results['result'] = 'skip'
			else:
				results['result'] = 'fail'
				out = self.interpretResult(out, returncode, results)

			crash_codes = [
				# Unix: terminated by a signal
				-5,  # SIGTRAP
				-6,  # SIGABRT
				-8,  # SIGFPE  (Floating point exception)
				-10, # SIGUSR1
				-11, # SIGSEGV (Segmentation fault)
				# Windows:
				# EXCEPTION_ACCESS_VIOLATION (0xc0000005):
				-1073741819,
				# EXCEPTION_INT_DIVIDE_BY_ZERO (0xc0000094):
				-1073741676
			]

			if returncode in crash_codes:
				results['result'] = 'crash'
			elif returncode != 0:
				results['note'] = 'Returncode was {0}'.format(returncode)

			if valgrind:
				# If the underlying test failed, simply report
				# 'skip' for this valgrind test.
				if results['result'] != 'pass':
					results['result'] = 'skip'
				elif returncode == 0:
					# Test passes and is valgrind clean.
					results['result'] = 'pass'
				else:
					# Test passed but has valgrind errors.
					results['result'] = 'fail'

			env = ''
			for key in self.env:
				env = env + key + '="' + self.env[key] + '" '
			if env:
				results['environment'] = env

			results['info'] = unicode("Returncode: {0}\n\nErrors:\n{1}\n\nOutput:\n{2}").format(returncode, err, out)
			results['returncode'] = returncode
			results['command'] = ' '.join(self.command)

			self.handleErr(results, err)

		else:
			results = TestResult()
			if 'result' not in results:
				results['result'] = 'skip'

		return results

	def check_for_skip_scenario(self, command):
		global PIGLIT_PLATFORM
		if PIGLIT_PLATFORM == 'gbm':
			if 'glean' == self.split_command:
				return True
			if self.split_command.startswith('glx-'):
				return True
		return False

	def get_command_result(self, command, fullenv):
		try:
			proc = subprocess.Popen(
				command,
				stdout=subprocess.PIPE,
				stderr=subprocess.PIPE,
				env=fullenv,
				universal_newlines=True
				)
			out, err = proc.communicate()
			returncode = proc.returncode
		except OSError as e:
			# Different sets of tests get built under
			# different build configurations.  If
			# a developer chooses to not build a test,
			# Piglit should not report that test as having
			# failed.
			if e.strerror == "No such file or directory":
				out = "PIGLIT: {'result': 'skip'}\n" \
				    + "Test executable not found.\n"
				err = ""
				returncode = None
			else:
				raise e
		return out, err, returncode

#############################################################################
##### PlainExecTest: Run a "native" piglit test executable
##### Expect one line prefixed PIGLIT: in the output, which contains a
##### result dictionary. The plain output is appended to this dictionary
#############################################################################
class PlainExecTest(ExecTest):
	def __init__(self, command):
		ExecTest.__init__(self, command)
		if os.environ.has_key("ANDROID_BUILD_TOP") == True and platform.system() == 'Linux':
			writepath = os.environ['HOME']
			self.command_name = command[0]
			dashloc = command[0].find('-')

			if dashloc != -1:
				foldername = command[0][0:dashloc]
			else:
				foldername = command[0]

			self.sh_script_remote = \
				("#!/system/bin/sh\n" + \
				"export PIGLIT_PLATFORM=android\n" + \
				"export PIGLIT_SOURCE_DIR=/data\n" + \
				"chmod 777 /system/bin/{command}\n" + \
				"/system/bin/{command} {param} 2> /data/piglittext_{command}.log\n" + \
				"echo $? > /data/piglitreturncode_{command}.log\n" + \
				"rm /system/bin/{command}\n").format(command=command[0], param=' '.join(command[1:]))

			self.sh_script_host = \
				("#!/bin/bash\n" + \
				"which adb &>/dev/null\n" + \
				"\n" + \
				"if [ $? == 1 ] ; then\n" + \
				"    echo \"no adb\" 1>&2\n" + \
				"    return 1\n" + \
				"    fi\n" + \
				"\n" + \
				"adb remount &>/dev/null\n" + \
				"\n" + \
				"for i in $*; do\n" + \
				"		if [[ \"$i\" == */lib/* ]]; then\n" + \
				"				 adb push $i /system/lib &>/dev/null\n" + \
				"		else\n" + \
				"				 adb push $i /system/bin &>/dev/null\n" + \
				"		fi\n" + \
				"done\n" + \
				"\n" + \
				"adb shell \"if [ ! -d \\\"/data/tests\\\" ]; then mkdir /data/tests; fi\" &>/dev/null\n" + \
				"adb shell \"if [ ! -d \\\"/data/tests/spec\\\" ]; then mkdir /data/tests/spec; fi\" &>/dev/null\n" + \
				"\n" + \
				"adb push {bindir}../tests/spec/{testdir} /data/tests/spec/{testdir} &>/dev/null\n" + \
				"\n" + \
				"adb shell source /system/bin/piglit_{command}_android.sh\n" + \
				"adb shell rm /system/bin/piglit_{command}_android.sh\n" + \
				"\n" + \
				"adb pull /data/piglittext_{command}.log /tmp &>/dev/null\n" + \
				"adb shell rm /data/piglittext_{command}.log\n" + \
				"adb pull /data/piglitreturncode_{command}.log /tmp &>/dev/null\n" + \
				"adb shell rm /data/piglitreturncode_{command}.log\n" + \
				"\n" + \
				"grep 'PIGLIT:' /tmp/piglittext_{command}.log 1>&2\n" + \
				"returncode=$(</tmp/piglitreturncode_{command}.log)\n" + \
				"rm /tmp/piglittext_{command}.log\n" + \
				"rm /tmp/piglitreturncode_{command}.log\n" + \
				"return $returncode\n").format(command=self.command_name, testdir=foldername, bindir=testBinDir)

			temp_command = ['/bin/bash', \
				'-c', \
				'source ' + writepath + '/piglit_{command}_host.sh'.format(command=self.command_name), \
				writepath + '/piglit_{command}_android.sh'.format(command=self.command_name), \
				writepath + '/piglit_{command}_android.sh'.format(command=self.command_name), \
				testBinDir + self.command[0], \
				testBinDir + '../lib/libpiglitutil_gles1.so', \
				testBinDir + '../lib/libpiglitutil_gles2.so', \
				testBinDir + '../lib/libpiglitutil.so' ]

			# this is to check if there are additional
			# files needed to run the test. If such files
			# found they will also be copied to device.
			for i in range(1,len(self.command)):
				if len(self.command[i]) >= 1  and self.command[i][0] != '-':
					temp_command.append(self.command[i])

			self.command = temp_command
		else:
			# Prepend testBinDir to the path.
			self.command[0] = testBinDir + self.command[0]

	def interpretResult(self, out, returncode, results):
		outlines = out.split('\n')
		outpiglit = map(lambda s: s[7:], filter(lambda s: s.startswith('PIGLIT:'), outlines))

		if len(outpiglit) > 0:
			try:
				for piglit in outpiglit:
					if piglit.startswith('subtest'):
						if not results.has_key('subtest'):
							results['subtest'] = {}
						results['subtest'].update(eval(piglit[7:]))
					else:
						results.update(eval(piglit))
				out = '\n'.join(filter(lambda s: not s.startswith('PIGLIT:'), outlines))
			except:
				results['result'] = 'fail'
				results['note'] = 'Failed to parse result string'

		if 'result' not in results:
			results['result'] = 'fail'
		return out
