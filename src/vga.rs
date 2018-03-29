use core::fmt;
use core::ptr::Unique;
use volatile::Volatile;
use spin::Mutex;

#[allow(dead_code)]
#[repr(u8)]
#[derive(Debug, Copy, Clone)]
pub enum Color {
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGray = 7,
    DarkGray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    Pink = 13,
    Yellow = 14,
    White = 15,
}

#[derive(Debug, Copy, Clone)]
struct ColorCode(u8);

impl ColorCode {
    const fn new(foreground: Color, background: Color) -> ColorCode {
        ColorCode((background as u8) << 4 | (foreground as u8))
    }
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
struct Char {
    ascii_char: u8,
    color: ColorCode,
}

const BUFFER_WIDTH: usize = 80;
const BUFFER_HEIGHT: usize = 25;

#[repr(C)]
struct CharBuffer {
    chars: [[Volatile<Char>; BUFFER_WIDTH]; BUFFER_HEIGHT],
}

pub struct Writer {
    column: usize,
    color: ColorCode,
    buffer: Unique<CharBuffer>,
}

impl Writer {
    pub fn write_byte(&mut self, byte: u8) {
        match byte {
            b'\n' => self.new_line(),
            byte => {
                if self.column >= BUFFER_WIDTH {
                    self.new_line();
                }

                let col = self.column;
                let row = BUFFER_HEIGHT - 1;

                let color_code = self.color;
                self.buffer().chars[row][col].write(Char {
                    ascii_char: byte,
                    color: color_code,
                });

                self.column += 1;
            }
        }
    }

    fn buffer(&mut self) -> &mut CharBuffer {
        unsafe { self.buffer.as_mut() }
    }

    fn new_line(&mut self) {
        for row in 1..BUFFER_HEIGHT {
            for col in 0..BUFFER_WIDTH {
                let buffer = self.buffer();
                let c = buffer.chars[row][col].read();

                buffer.chars[row - 1][col].write(c);
            }
        }

        self.column = 0;
        let row = BUFFER_HEIGHT - 1;
        let color = self.color;

        for col in 0..BUFFER_WIDTH {
            self.buffer().chars[row][col].write(Char {
                ascii_char: b' ',
                color: color,
            });
        }
    }
}

impl fmt::Write for Writer {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        for b in s.bytes() {
            self.write_byte(b);
        }

        Ok(())
    }
}

const VGA_BASE_ADDR: usize = 0xb_8000;

pub static WRITER: Mutex<Writer> = Mutex::new(Writer {
    column: 0,
    color: ColorCode::new(Color::LightGreen, Color::Black),
    buffer: unsafe { Unique::new_unchecked(VGA_BASE_ADDR as *mut _) },
});

macro_rules! println {
    ($fmt:expr) => (print!(concat!($fmt, "\n")));
    ($fmt:expr, $($arg:tt)*) => (print!(concat!($fmt, "\n"), $($arg)*));
}

macro_rules! print {
    ($($arg:tt)*) => ({
        use core::fmt::Write;
        let mut writer = $crate::vga::WRITER.lock();
        writer.write_fmt(format_args!($($arg)*)).unwrap();
    });
}

pub fn clear() {
    for i in 0..BUFFER_HEIGHT {
        println!("");
    }
}