#![feature(lang_items)]
#![no_std]

extern crate rlibc;

#[no_mangle]
pub extern fn rust_main() {
    unsafe {
        let vga_base_addr = 0xb_8000usize;
        let vga: &mut [u8; 80*25*2] = &mut *(vga_base_addr as *mut [u8; 80*25*2]);
        let text = "heh ebin";
        let mut idx = 160usize;
        for &c in text.as_bytes() {
            vga[idx] = c;
            idx += 2;
        }
    }
}

#[lang = "eh_personality"] #[no_mangle] pub extern fn eh_personality() {}
#[lang = "panic_fmt"] #[no_mangle] pub extern fn panic_fmt() -> ! {loop{}}