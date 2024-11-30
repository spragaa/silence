#!/usr/bin/env python3

import subprocess
import os
import xml.etree.ElementTree as ET
from typing import List, Dict
from dataclasses import dataclass
import sys
from colorama import init, Fore, Style

@dataclass
class TestResult:
    suite_name: str
    total_tests: int
    failed_tests: int
    passed_tests: int
    execution_time: float
    failed_test_names: List[str]
    output: str
    exists: bool = True

class TestRunner:
    def __init__(self):
        self.test_suites = [
            "../build/tests/test_server/test_server",
            "../build/tests/test_common/test_common",
            "../build/tests/test_crypto/test_crypto",
            "../build/tests/test_file_server/test_file_server"
        ]
        self.results_path = "../build/test_results.xml"
        init(strip=False)

    def run_test_suite(self, suite_path: str) -> TestResult:
        if not os.path.isfile(suite_path):
            return TestResult(
                suite_name=suite_path,
                total_tests=0,
                failed_tests=0,
                passed_tests=0,
                execution_time=0.0,
                failed_test_names=[],
                output="",
                exists=False
            )

        try:
            process = subprocess.Popen(
                [suite_path, f"--gtest_output=xml:{self.results_path}"],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                bufsize=1,
                universal_newlines=True,
                errors='replace'
            )

            output = []
            while True:
                line = process.stdout.readline()
                if not line and process.poll() is not None:
                    break
                if line:
                    output.append(line)
                    print(line, end='', flush=True)

            stdout, stderr = process.communicate()
            if stdout:
                print(stdout, end='', flush=True)
                output.append(stdout)
            if stderr:
                print(f"{Fore.RED}{stderr}{Style.RESET_ALL}", end='', flush=True)

            output_text = ''.join(output)

            try:
                tree = ET.parse(self.results_path)
                root = tree.getroot()

                total_tests = 0
                failed_tests = 0
                failed_test_names = []
                execution_time = 0.0

                for testsuite in root.findall(".//testsuite"):
                    total_tests += int(testsuite.get("tests", 0))
                    failed_tests += int(testsuite.get("failures", 0))
                    execution_time += float(testsuite.get("time", 0))

                    for testcase in testsuite.findall(".//testcase"):
                        if testcase.find("failure") is not None:
                            failed_test_names.append(
                                f"{testcase.get('classname')}.{testcase.get('name')}"
                            )

                return TestResult(
                    suite_name=suite_path,
                    total_tests=total_tests,
                    failed_tests=failed_tests,
                    passed_tests=total_tests - failed_tests,
                    execution_time=execution_time,
                    failed_test_names=failed_test_names,
                    output=output_text
                )

            except ET.ParseError:
                total_tests = output_text.count("[----------]")
                failed_tests = output_text.count("[  FAILED  ]") - 1
                if failed_tests < 0:
                    failed_tests = 0

                return TestResult(
                    suite_name=suite_path,
                    total_tests=total_tests,
                    failed_tests=failed_tests,
                    passed_tests=total_tests - failed_tests,
                    execution_time=0.0,
                    failed_test_names=[],
                    output=output_text
                )

        except Exception as e:
            print(f"{Fore.RED}Error running test suite {suite_path}: {e}{Style.RESET_ALL}")
            return TestResult(
                suite_name=suite_path,
                total_tests=0,
                failed_tests=0,
                passed_tests=0,
                execution_time=0.0,
                failed_test_names=[],
                output=str(e),
                exists=True
            )

    def print_results(self, results: List[TestResult]):
        print("\nTest Summary:")
        print("-------------")

        total_failed = 0
        total_passed = 0
        total_tests = 0
        total_time = 0.0

        for result in results:
            if not result.exists:
                print(f"{Fore.RED}Test suite {result.suite_name} not found.{Style.RESET_ALL}")
                continue

            status_color = Fore.RED if result.failed_tests > 0 else Fore.GREEN
            print(f"{status_color}Test suite: {result.suite_name}")
            print(f"  Total tests: {result.total_tests}")
            print(f"  Passed: {result.passed_tests}")
            print(f"  Failed: {result.failed_tests}")
            print(f"  Execution time: {result.execution_time:.2f}s{Style.RESET_ALL}")

            if result.failed_tests > 0:
                print(f"{Fore.RED}Failed tests:")
                for test_name in result.failed_test_names:
                    print(f"  - {test_name}")
                print(Style.RESET_ALL)

            total_failed += result.failed_tests
            total_passed += result.passed_tests
            total_tests += result.total_tests
            total_time += result.execution_time

        print("\nOverall Summary:")
        print("----------------")
        print(f"{Fore.WHITE}Total Tests: {total_tests}")
        print(f"{Fore.GREEN}Total Passed: {total_passed}")
        print(f"{Fore.RED}Total Failed: {total_failed}")
        print(f"{Fore.WHITE}Total Execution Time: {total_time:.2f}s{Style.RESET_ALL}")

    def run(self):
            print("Running tests...")
            print("----------------")
    
            results = []
            for suite in self.test_suites:
                print(f"\nExecuting {suite}...")
                result = self.run_test_suite(suite)
                results.append(result)
    
            self.print_results(results)
    
            total_failed = sum(r.failed_tests for r in results)
            if total_failed > 0:
                sys.exit(1)
    
            sys.exit(0)
            
if __name__ == "__main__":
    runner = TestRunner()
    runner.run()