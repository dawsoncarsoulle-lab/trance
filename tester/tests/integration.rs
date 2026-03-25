use std::path::Path;
use std::process::Command;

#[test]
fn tester_output() {
    let exe = Path::new("../build/facile");
    let code = Path::new("../code.fac");
    let output = Command::new(exe).arg(code).output().expect("non trouve");
    let stdout = String::from_utf8_lossy(&output.stdout);
    let mut count_line: u32 = 0;
    for ligne in stdout.lines().enumerate() {
        let line = ligne.1.trim();
        if line.is_empty() {
            continue;
        }
        count_line += 1;

        if !line.starts_with("TOK") {
            panic!("erreur ligne{} : {}", ligne.0 + 1, ligne.1)
        }
        println!("{}", count_line);
    }
}
