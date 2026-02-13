line_number = 1
has_error = False
with open("check", "r") as check_file:
    lines = check_file.readlines()
    for line in lines:
        if not line.startswith("TOK"):
            print(f"ERROR line {line_number}: {line}")
            has_error = True
        line_number += 1

if not has_error:
    print("SUCCESS")
