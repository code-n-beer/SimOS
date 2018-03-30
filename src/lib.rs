#![feature(lang_items)]
#![feature(unique)]
#![feature(const_fn)]
#![feature(ptr_internals)]
#![no_std]

extern crate rlibc;
extern crate volatile;
extern crate spin;
extern crate multiboot2;

#[macro_use]
mod vga;

#[no_mangle]
pub extern fn rust_main(multiboot_addr: usize) {
    vga::clear();

    let multiboot = unsafe {
        multiboot2::load(multiboot_addr)
    };

    let memory_map = multiboot.memory_map_tag()
        .expect("no memory info in multiboot struct");
    
    println!("Memory map:");
    for area in memory_map.memory_areas() {
        println!("    {:#016x}-{:#016x}", area.base_addr, area.base_addr + area.length);
    }

    let elf_sections = multiboot.elf_sections_tag()
        .expect("no elf sections in multiboot struct");
    
    println!("Sections:");
    for section in elf_sections.sections() {
        println!("    {:#016x}: size={:#08x}, flags={:#04x}", section.addr,
            section.size, section.flags);
    }
}

#[lang = "eh_personality"] #[no_mangle] pub extern fn eh_personality() {}

#[lang = "panic_fmt"]
#[no_mangle]
pub extern fn panic_fmt(fmt: core::fmt::Arguments, file: &'static str, line: u32) -> ! {
    println!("\n\nPANIC in {} at line {}:", file, line);
    println!("    {}", fmt);
    loop {}
}