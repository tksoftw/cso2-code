import subprocess

T = 10
for scenario_number in range(1, 9):
    total_time = 0.0
    for _ in range(T):
        total_time += float(subprocess.check_output(["./gettimings", str(scenario_number)]))
    average_time = total_time / T
    print(f"Scenario {scenario_number}: {average_time} ns")