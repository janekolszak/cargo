from xml.dom import minidom
import sys



BLACK = "\033[30m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
CYAN = "\033[36m"
WHITE = "\033[37m"
BOLD = "\033[1m"
ENDC = "\033[0m"



class Logger(object):
    __failedTests = []

    # Create summary of test providing DOM object with parsed XML
    def info(self, msg):
        print BOLD + msg + ENDC

    def infoTitle(self, msg):
        print CYAN + BOLD + msg + ENDC

    def error(self, msg):
        print RED + BOLD + msg + ENDC

    def success(self, msg):
        print GREEN + BOLD + msg + ENDC


    __indentChar = "    "
    def testCaseSummary(self, testSuite, testName, testResult, recLevel):
        msg = self.__indentChar * recLevel + BOLD + "{:<50} ".format(testName)

        if testResult == "passed":
            msg += GREEN
        else:
            msg += RED
            self.__failedTests.append(testSuite + "/" + testName)

        print msg + testResult + ENDC

    def testSuiteSummary(self, suite, recLevel=0, summarize=False):
        indPrefix = self.__indentChar * recLevel

        self.infoTitle(indPrefix + suite.attributes["name"].value + " results:")

        for child in suite.childNodes:
            if child.nodeName == "TestSuite":
                self.testSuiteSummary(child, recLevel=recLevel + 1)
            elif child.nodeName == "TestCase":
                self.testCaseSummary(suite.attributes["name"].value,
                                     child.attributes["name"].value,
                                     child.attributes["result"].value,
                                     recLevel=recLevel + 1)

        if summarize:
            self.infoTitle(indPrefix + suite.attributes["name"].value + " summary:")
            self.info(indPrefix + "Passed tests: " + suite.attributes["test_cases_passed"].value)
            self.info(indPrefix + "Failed tests: " + suite.attributes["test_cases_failed"].value +
                      '\n')

    def XMLSummary(self, dom):
        self.info("\n=========== SUMMARY ===========\n")

        for result in dom.getElementsByTagName("TestResult"):
            for child in result.childNodes:
                if child.nodeName == "TestSuite":
                    self.testSuiteSummary(child, summarize=True)

    def failedTestSummary(self, bin):
        if not self.__failedTests:
            return

        commandPrefix = "cargo_launch_test.py " + bin + " -t "
        self.infoTitle("Some tests failed. Use following command(s) to launch them explicitly:")
        for test in self.__failedTests:
            self.error(self.__indentChar + commandPrefix + "'{0}'".format(test))

    def terminatedBySignal(self, bin, signum):
        self.error("\n=========== FAILED ===========\n")
        signame = { 2: "SIGINT (Interrupt)",
                    3: "SIGQUIT (Quit)",
                    5: "SIGTRAP (Trace trap)",
                    6: "SIGABRT (Abort)",
                    8: "SIGFPE (Floating-point exception)",
                    9: "SIGKILL (Kill)",
                   11: "SIGSEGV (Segmentation fault)",
                   15: "SIGTERM (Termination)"}
        siginfo = signame.get(signum, 'signal ' + str(signum))
        self.error('Terminated by ' + siginfo)
        if signum in [5, 6, 8, 11]:
            self.error("\nUse following command to launch debugger:")
            self.error(self.__indentChar + "cargo_launch_test.py --gdb " + bin)


class Parser(object):
    _testResultBegin = "<TestResult>"
    _testResultEnd = "</TestResult>"

    def __parseAndWriteLine(self, line):
        result = ""
        # Entire XML is kept in one line, if line begins with <TestResult>, extract it
        if self._testResultBegin in line:
            result += line[line.find(self._testResultBegin):
                           line.find(self._testResultEnd) + len(self._testResultEnd)]
            line = line[0:line.find(self._testResultBegin)] + line[line.find(self._testResultEnd) +
                                                                   len(self._testResultEnd):]
        sys.stdout.write(line)
        sys.stdout.flush()

        return result


    def parseOutputFromProcess(self, p):
        """Parses stdout from given subprocess p.
        """
        testResult = ""
        # Dump test results
        while True:
            outline = p.stdout.readline()
            testResult += self.__parseAndWriteLine(outline)
            # Break if process has ended and everything has been read
            if not outline and p.poll() != None:
                break

        return testResult
