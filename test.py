import sys
import os
import random
import subprocess
from datetime import datetime
from tqdm import tqdm

class BPlusTreeTester:
    def __init__(self):
        self.test_cases = []
        self.result_log = []
        self.executable = "./main"  # 编译后的可执行文件路径
        self.test_data_dir = "test_data"
        self.result_dir = "test_results"
        self.current_test = 0
        
        # 创建测试目录
        os.makedirs(self.test_data_dir, exist_ok=True)
        os.makedirs(self.result_dir, exist_ok=True)

    def generate_test_case(self, case_type, size=1000):
        """生成不同类型测试用例"""
        base_str = "key_"
        values = list(range(100000, 100000 + size))
        random.shuffle(values)
        
        operations = []
        expected = {}
        
        if case_type == "sequential":
            # 顺序插入测试
            for i in range(size):
                key = f"{base_str}{i:05d}"
                val = values[i]
                operations.append(f"insert {key} {val}")
                expected.setdefault(key, []).append(val)
        
        elif case_type == "random":
            # 随机操作测试
            for _ in range(size):
                if random.random() < 0.7:  # 70%插入
                    key = f"{base_str}{random.randint(0, size//10):05d}"
                    val = random.randint(1, 1000000)
                    operations.append(f"insert {key} {val}")
                    expected.setdefault(key, []).append(val)
                    expected[key] = list(sorted(set(expected[key])))
                else:  # 30%查找
                    key = f"{base_str}{random.randint(0, size//10):05d}"
                    operations.append(f"find {key}")
        
        elif case_type == "delete":
            # 删除测试
            inserted = {}
            for _ in range(size):
                key = f"{base_str}{random.randint(0, size//2):05d}"
                val = random.randint(1, 1000000)
                operations.append(f"insert {key} {val}")
                inserted.setdefault(key, set()).add(val)
            
            for key in list(inserted.keys()):
                if inserted[key]:
                    val = random.choice(list(inserted[key]))
                    operations.append(f"delete {key} {val}")
                    inserted[key].remove(val)
                    if not inserted[key]:
                        del inserted[key]
            
            # 构建预期结果
            expected = {k: sorted(v) for k, v in inserted.items()}
        
        return {
            "operations": operations,
            "expected": expected,
            "type": case_type
        }

    def generate_all_cases(self):
        """生成完整测试套件"""
        test_types = [
            ("small_seq", "sequential", 10),
            ("medium_random", "random", 10),
            ("large_delete", "delete", 10),
            ("edge_case1", "edge", 10)
        ]
        
        for name, ttype, size in test_types:
            if ttype == "edge":
                case = self.generate_edge_case()
            else:
                case = self.generate_test_case(ttype, size)
            self.test_cases.append((name, case))

    def generate_edge_case(self):
        """生成边界测试用例"""
        return {
            "operations": [
                "insert empty_key 123",
                "insert long_key_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa 456",
                "find non_exist",
                "delete non_exist 789",
                "insert dup_key 111",
                "insert dup_key 111",  # 重复插入
                "find dup_key",
                "delete dup_key 111",
                "find dup_key"
            ],
            "expected": {
                "empty_key": [123],
                "long_key_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa": [456],
                "dup_key": []
            },
            "type": "edge"
        }

    def run_test(self, case_name, test_case):
        """执行单个测试用例"""
        test_file = os.path.join(self.test_data_dir, f"{case_name}.in")
        result_file = os.path.join(self.result_dir, f"{case_name}.out")
        
        # 生成测试文件
        with open(test_file, "w") as f:
            f.write(f"{len(test_case['operations'])}\n")
            f.write("\n".join(test_case['operations']))
        
        # 运行程序
        try:
            with open(test_file) as fin, open(result_file, "w") as fout:
                subprocess.run(
                    [self.executable],
                    stdin=fin,
                    stdout=fout,
                    stderr=subprocess.PIPE,
                    check=True,
                    timeout=10
                )
        except subprocess.TimeoutExpired:
            return False, "Timeout"
        except subprocess.CalledProcessError as e:
            return False, f"Runtime Error: {e.stderr.decode()}"
        
        return True, result_file

    def verify_results(self, result_file, expected):
        """验证测试结果"""
        actual = {}
        with open(result_file) as f:
            for line in f:
                line = line.strip()
                if line == "null":
                    continue
                if line.startswith("find"):
                    parts = line.split()
                    key = parts[1]
                    values = list(map(int, parts[2:]))
                    actual[key] = values
        
        # 验证查找结果
        errors = []
        for key in expected:
            exp = sorted(expected[key])
            act = sorted(actual.get(key, []))
            if exp != act:
                errors.append(f"Key {key} mismatch: Expected {exp}, Got {act}")
        
        # 检查多余结果
        for key in actual:
            if key not in expected:
                errors.append(f"Unexpected key found: {key}")
        
        return errors

    def run_all_tests(self):
        """执行完整测试套件"""
        print(f"Starting B+ Tree Test Suite at {datetime.now()}")
        print("="*60)
        
        total = len(self.test_cases)
        passed = 0
        failed_cases = []
        
        for name, case in self.test_cases:
            self.current_test += 1
            print(f"[Test {self.current_test}/{total}] {name} ({case['type']})...")
            
            # 执行测试
            success, result = self.run_test(name, case)
            if not success:
                print(f"  Failed! Reason: {result}")
                failed_cases.append((name, result))
                continue
            
            # 验证结果
            errors = self.verify_results(result, case['expected'])
            if not errors:
                print("  Passed!")
                passed += 1
            else:
                print(f"  Failed with {len(errors)} errors:")
                for err in errors[:3]:  # 显示前3个错误
                    print(f"  - {err}")
                failed_cases.append((name, "\n".join(errors)))
            
            print("-"*60)
        
        # 生成报告
        print("\nTest Summary:")
        print(f"Total Cases: {total}")
        print(f"Passed: {passed} ({passed/total*100:.1f}%)")
        print(f"Failed: {total - passed}")
        if failed_cases:
            print("\nFailed Cases Details:")
            for name, reason in failed_cases:
                print(f"{name}:")
                print(f"  {reason[:100]}...")  # 显示部分错误信息
        
        # 保存详细日志
        log_file = os.path.join(self.result_dir, "test_report.log")
        with open(log_file, "w") as f:
            f.write("\n".join(self.result_log))
        
        print(f"\nDetailed log saved to {log_file}")

if __name__ == "__main__":
    tester = BPlusTreeTester()
    tester.generate_all_cases()
    tester.run_all_tests()