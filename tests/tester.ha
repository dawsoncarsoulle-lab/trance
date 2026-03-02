use fmt;
use io;
use os;
use os::exec;
use strings;
use time;

// This is needed because otherwise we read to fast and the mono runtime hasn't written to the pipe yet
const MONO_RUNTIME_STARTUP_COST = 100000000;

// This is the base buiding block, ALL tests need to be at least assembled
fn assemble(filename:  str)  void = {
    let generate_ir_cmd_str = fmt::asprintf("../build/facile < {}", filename) as str;
    defer free(generate_ir_cmd_str);

    let generate_ir_cmd = exec::cmd("sh", "-c", generate_ir_cmd_str)!;
    let generate_ir_process = exec::start(&generate_ir_cmd)!;
    exec::wait(&generate_ir_process)!;

    let assembler_cmd = exec::cmd("ilasm", "facile.il")!;
    let assembler_process = exec::start(&assembler_cmd)!;
    exec::wait(&assembler_process)!;
};


fn non_interactive_test(filename: str) str = {

    assemble(filename);

    let runner_cmd = exec::cmd("mono", "facile.exe")!;
    let pipe = exec::pipe();
    exec::addfile(&runner_cmd, os::stdout_file, pipe.1)!;

    let runner_process = exec::start(&runner_cmd)!;
    io::close(pipe.1)!;

    time::sleep(MONO_RUNTIME_STARTUP_COST);
    let test_output: [2048]u8 = [0...];
    let n = match (io::read(pipe.0, &test_output)!) {
        case let s: size => yield s;
        case io::EOF => yield 0z;
    };
    exec::wait(&runner_process)!;

    let text = strings::fromutf8(test_output[..n]) as str;
    return strings::dup(text) as str;

};

fn interactive_test(filename: str, input: str...) str = {
    assemble(filename);

    let runner_cmd = exec::cmd("mono", "facile.exe")!;

    let in_pipe = exec::pipe();
    let out_pipe = exec::pipe();

    exec::addfile(&runner_cmd, os::stdin_file, in_pipe.0)!;
    exec::addfile(&runner_cmd, os::stdout_file, out_pipe.1)!;

    let runner_process = exec::start(&runner_cmd)!;
    io::close(in_pipe.0)!;
    io::close(out_pipe.1)!;
    io::writeall(in_pipe.1, strings::toutf8(strings::join("\n",input...) as str))!;
    io::close(in_pipe.1)!;

    time::sleep(MONO_RUNTIME_STARTUP_COST);
    let test_output: [2048]u8 = [0...];
    let n = match (io::read(out_pipe.0, &test_output)!) {
        case let s: size => yield s;
        case io::EOF => yield 0z;
    };

    exec::wait(&runner_process)!;

    let text = strings::fromutf8(test_output[..n]) as str;
    return strings::dup(text) as str;
};

@fini fn cleanup() void = {
    os::remove("facile.exe")!;
    os::remove("facile.il")!;
};

fn assert_answer(output : str , answer : str) void = {
    fmt::printfln("DEBUG: I received {}", strings::trim(output))!;
    assert(strings::trim(output) == answer, fmt::asprintf("Output did not match {}, was {}", answer, output) as str);
};

fn assert_answers(output: str, answers: str...) void = {
    let clean_out = strings::trim(output);
    fmt::printfln("DEBUG: I received {}", clean_out)!;
    let actual_lines = strings::split(clean_out, "\n") as []str;

    for (let i = 0z; i < len(answers); i += 1) {
        let actual = strings::trim(actual_lines[i]);
        let expected = answers[i];
        assert_answer(expected, actual);
    };
};

@test fn test_math() void = {
    let out = non_interactive_test("test-math");
    assert_answer(out, "25");
    free(out);
};

@test fn test_read_empty() void = {
    let out = interactive_test("test-read_empty", "0");
    assert_answer(out, "0");
    free(out);
};

@test fn test_read_math() void = {
    let out = interactive_test("test-read_math", "20");
    assert_answer(out, "30");
    free(out);
};

@test fn test_read_expr_precedence() void = {
    let out = non_interactive_test("test-read_expr_precedence");
    assert_answers(out, "50", "60", "10");
    free(out);
};

fn test_if_condition() void = {
    let out = non_interactive_test("test-if_condition");
    assert_answers(out, "5");
    free(out);
};

fn test_if_else() void = {
    let out = non_interactive_test("test-if_else");
    assert_answers(out, "5", "3");
    free(out);
};

@test fn test_while_loop() void = {
    let out = non_interactive_test("test-while_loop");
    assert_answers(out, "0","1","2","3","4","5","6","7","8","9");
    free(out);
};

@test fn test_while_loop_nested() void = {
    let out = non_interactive_test("test-while_loop_nested");
    assert_answers(out, "5","5","4","3","2","1","4","3","2","1");
    free(out);
};


@test fn test_largest_common_denominator() void = {
    let out = interactive_test("test-largest_common_denominator", "387", "129");
    assert_answer(out, "129");
    free(out);
};
