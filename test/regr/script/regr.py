#!/usr/bin/env python
import yaml
import sys
import os
import logging

cleanLog = False

def printHelp():
    """regr [test_case] | [-f <test_list>] | [-h] | [-c]"""
    print(printHelp.__doc__)

def parseTestList(tests, fname):
    try:
        fp = open(fname, "r")
    except:
        logging.error("Can't open file {}".format(fname))
        sys.exit(-1);
    dname = os.path.dirname(fname)
    while True:
        line = fp.readline()
        if not line:
            break
        else:
            line = line.rstrip()
            if line[0] == "/":
                tests.append(line)
            else:
                tests.append(os.path.join(dname, line))
            continue
    fp.close()

def parseArgv():
    argv = sys.argv
    tests = []
    flists = []
    i = 1
    global cleanLog
    while i < len(argv):
        if argv[i] == "-h":
            printHelp()
        elif argv[i] == "-c":
            cleanLog = True
        elif argv[i] == "-f":
            if i + 1 == len(argv):
                logging.error("No argument follows '-f'")
                sys.exit(-1);
            i = i + 1
            parseTestList(tests, argv[i])
        else:
            tests.append(argv[i])
        i = i + 1
    return tests

def runTest(fpyaml, fname):
    test = yaml.safe_load(fpyaml)
    stages = ("run", "check", "clean")
    logs = []
    for stage in stages:
        if stage in test:
            log = "{}.{}.log".format(fname, stage)
            logs.append(log)
            cmd = test[stage] + " 2>&1 > " + log
            if stage == "clean" and not cleanLog:
                continue
            if os.system(cmd):
                return False
    if cleanLog:
        cmd = "rm -rf "
        for log in logs:
            cmd = cmd + log + " "
        os.system(cmd)
    return True

def runTests(tests):
    cwd = os.getcwd()
    failed = []
    for test in tests:
        try:
            fpyaml = open(test)
        except:
            logging.error("Can't open test case {}".format(test))
            failed.append(test)
            continue
        dname = os.path.dirname(test)
        fname = os.path.basename(test)
        fname = fname.split(".")[0]
        os.chdir(dname)
        if not runTest(fpyaml, fname):
            failed.append(test)
        os.chdir(cwd)

    logging.info("{} test cases run, {} passed, {} failed".format(len(tests), len(tests)-len(failed), len(failed)))
    if failed:
        logging.info("Failed test cases:")
        for test in failed:
            logging.info("    {}".format(test))
            
if __name__ == "__main__":
    logging.getLogger().setLevel(logging.INFO)
    logging.basicConfig(format="[%(levelname)-5s]: %(message)s")
    tests = parseArgv()
    runTests(tests)
