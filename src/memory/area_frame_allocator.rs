use memory::{Frame, FrameAllocator};
use multiboot2::{MemoryArea, MemoryAreaIter};

pub struct AreaFrameAllocator {
    next_free_frame: Frame,
    current_area: Option<&'static MemoryArea>,
    areas: MemoryAreaIter,
    kernel_start: Frame,
    kernel_end: Frame,
    multiboot_start: Frame,
    multiboot_end: Frame,
}

impl AreaFrameAllocator {
    fn is_kernel_frame(&self, frame: &Frame) -> bool {
        frame >= &self.kernel_start && frame <= &self.kernel_end
    }

    fn is_multiboot_frame(&self, frame: &Frame) -> bool {
        frame >= &self.multiboot_start && frame <= &self.multiboot_end
    }

    fn choose_next_area(&mut self) {
        self.current_area = self.areas
            .clone()
            .filter(|area| {
                let address = area.base_addr + area.length - 1;
                Frame::from_physical_address(address as usize) >= self.next_free_frame
            })
            .min_by_key(|area| area.base_addr);

        if let Some(area) = self.current_area {
            let start_frame = Frame::from_physical_address(area.base_addr as usize);
            if self.next_free_frame < start_frame {
                self.next_free_frame = start_frame;
            }
        }
    }

    pub fn new(
        kernel_start: usize,
        kernel_end: usize,
        multiboot_start: usize,
        multiboot_end: usize,
        memory_areas: MemoryAreaIter,
    ) -> AreaFrameAllocator {
        let mut allocator = AreaFrameAllocator {
            next_free_frame: Frame::from_physical_address(0),
            current_area: None,
            areas: memory_areas,
            kernel_start: Frame::from_physical_address(kernel_start),
            kernel_end: Frame::from_physical_address(kernel_end),
            multiboot_start: Frame::from_physical_address(multiboot_start),
            multiboot_end: Frame::from_physical_address(multiboot_end),
        };
        allocator.choose_next_area();
        allocator
    }
}

impl FrameAllocator for AreaFrameAllocator {
    fn allocate_frame(&mut self) -> Option<Frame> {
        if let Some(area) = self.current_area {
            let frame = Frame {
                number: self.next_free_frame.number,
            };

            let current_area_last_frame = {
                let last_frame_addr = area.base_addr + area.length - 1;
                Frame::from_physical_address(last_frame_addr as usize)
            };

            if frame > current_area_last_frame {
                // next area
            } else if self.is_kernel_frame(&frame) {
                self.next_free_frame = Frame {
                    number: self.kernel_end.number + 1,
                };
            } else if self.is_multiboot_frame(&frame) {
                self.next_free_frame = Frame {
                    number: self.multiboot_end.number + 1,
                };
            } else {
                self.next_free_frame.number += 1;
                return Some(frame);
            }

            self.allocate_frame()
        } else {
            None
        }
    }

    fn deallocate_frame(&mut self, frame: Frame) {
        unimplemented!()
    }
}
