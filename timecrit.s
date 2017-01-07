
 section .text:CODE
  PUBLIC WritePixels
  ;(uint16_t pixel, uint32_t nPixels, uint32_t GPIOx_BASE);
WritePixels
;r0 = pixel;
;r1 = nPixels;
;r2 = GPIOx Base
;r5 - value to write bsrr
  ldr  r2, =0x48000800/*GPIOC_BASE*/
  cmp           r1, #0
  beq           _exit1
  push {r4-r6}
  movs  r4, #255 ;LSB in r4
  ands  r4, r4, r0
  lsrs r0, r0, #8 ;MSB in r0
  movs r5, #1  ;load BSRR value
  lsls r5, r5, #8
  
  movs r6, #0
  mvns r6, r6 ; -1 in r6
_lp1  
  strh          r0, [r2, #0x14];write odr
  strh          r5,[r2, #0x18]
  adds          r1, r1, r6
  strh          r4, [r2, #0x14]
  strh          r5, [r2, #0x18]
  bne           _lp1  
  
  pop {r4-r6}
_exit1
  bx      lr
;-------------------------------------------------------------------------------  
  
  PUBLIC WritePixelsBitmap
WritePixelsBitmap       ;(uint16_t* bm, uint32_t nPixels, uint32_t GPIOx_BASE);
  
  ldr           r2, =0x48000800/*GPIOC_BASE*/
  push          {r4}
  movs          r3, #1                  ;load BSRR value
  lsls          r3, r3, #8
  
_lp2
  ldrb          r4,[r0, #1]
  strh          r4, [r2, #0x14]         ;write odr
  strh          r3,[r2, #0x18]
  ldrb          r4, [r0]
  strh          r4, [r2, #0x14]
  strh          r3, [r2, #0x18]
  adds          r0, r0, #2
  subs          r1, r1, #1
  bne           _lp2
  
  pop           {r4}  
  bx lr
  
;------------------------------------------------------------------------------- 
  PUBLIC WritePixelsBitmap2
WritePixelsBitmap2       ;(uint16_t* bm, uint32_t nPixels, uint32_t GPIOx_BASE);
  
  push          {r4-r6}
  movs          r3, #1                  ;load BSRR value
  movs          r6, #255                ;mask
  lsls          r3, r3, #8
  
  lsrs          r1, r1, #1              ;count uint32
  
_lp3
  ldr           r5,[r0]
  
  lsrs          r4, r5, #8
  ands          r4, r6
  strh          r4, [r2, #0x14]         ;write odr
  strh          r3,[r2, #0x18]
  
  movs          r4, r5
  ands          r4, r6
  strh          r4, [r2, #0x14]
  strh          r3, [r2, #0x18]
  
  lsrs          r4, r5, #24
  ands          r4, r6
  strh          r4, [r2, #0x14]
  strh          r3, [r2, #0x18]
  
  lsrs          r4, r5, #16
  ands          r4, r6
  strh          r4, [r2, #0x14]
  strh          r3, [r2, #0x18]
  
  adds          r0, r0, #4
  subs          r1, r1, #1
  bne           _lp3
  
  pop           {r4-r6}  
  bx lr

;-------------------------------------------------------------------------------

  PUBLIC DrawPixelSequenceFull_Fast
;(uint8_t* seq, uint32_t seqSize, uint16_t* clut, uint32_t GPIOx_BASE)
DrawPixelSequenceFull_Fast
  push          {r4-r7}
;loop counter r8,r9
  subs          r1, #1
  mov           r8, r1
  movs          r1, #0
  mvns          r1, r1
  mov           r9, r1

;write pulse BSRR in r7  
  movs          r7, #1
  lsls          r7, r7, #8
  
  
_lp_main  
  ldrb          r1, [r0]        ;read byte in r1
    
  cmp           r1, #254  
  BGE           _repeat
  
  lsls          r1, r1, #1      ;clut word offset in r1

  ldrh          r4, [r2, r1]    ; color word in r4
  
  lsrs          r5, r4, #8      ; High byte in r5
  movs          r1, #0xFF
  ands          r4, r1, r4      ; Low byte in r4
  
  strh          r5, [r3, #0x14]
  strh          r7, [r3, #0x18]
  
  strh          r4, [r3, #0x14]
  strh          r7, [r3, #0x18]
 
  adds          r0, r0, #1      ; next byte
  add           r8, r9          ; loop cnt--
  cmp           r8, r9
  bne           _lp_main
  b             _exit
  
_repeat
  adds          r0, r0, #1      ; next byte
  add           r8, r9          ; loop cnt--
  
  ldrb          r6, [r0]        ; Repeat count Low Byte in r6
  cmp           r1, #254        ; if 1-byte value
  bgt           _rep1           ; begin repeat
                                ; else load High Byte:
  adds          r0, #1          ; next byte
  add           r8, r9          ; loop cnt--
  ldrb          r1, [r0]
  lsls          r1, r1, #8
  orrs          r6, r6, r1
  
_rep1
  strh          r5, [r3, #0x14]
  strh          r7, [r3, #0x18]
  
  strh          r4, [r3, #0x14]
  strh          r7, [r3, #0x18]
  
  subs          r6, #1
  bne           _rep1           ; while(--repeatCnt > 0)
  

  adds          r0, r0, #1      ; next byte
  add           r8, r9          ; loop cnt--
  
  cmp           r8, r9
  bne           _lp_main
_exit
  pop {r4-r7}
  bx lr
  
;-------------------------------------------------------------------------------

  PUBLIC WritePixel
  ;(uint16_t pixel, uint32_t nPixels, uint32_t GPIOx_BASE);
WritePixel
;r0 = pixel;
;r1
;r2 = GPIOx Base
;r3 - value to write bsrr
  ldr  r2, =0x48000800/*GPIOC_BASE*/
  ldrh r0, [r0]
  movs  r1, #255 ;LSB in r4
  ands  r1, r1, r0
  lsrs r0, r0, #8 ;MSB in r0
  movs r3, #1  ;load BSRR value
  lsls r3, r3, #8
  
  strh          r0, [r2, #0x14];write odr
  strh          r3,[r2, #0x18]
  strh          r1, [r2, #0x14]
  strh          r3, [r2, #0x18] 
  

  bx            lr
//-----------------------------------------------------------------------------
  PUBLIC WriteComA/* (uint8_t com) */
WriteComA
  ldr           r1, =0x48000800 /* GPIOC_BASE */
  ldr           r2, =0x48000400 /* GPIOB_BASE */
  movs          r3, #2          /* GPIO_Pin_1 */
  strh          r3, [r2, #0x28] /* Reset D/C */
  strh          r0, [r1, #0x14] /* write odr */
  //movs          r0, #1
  lsls          r3, r3, #7      /* GPIO_Pin_8 */
  strh          r3, [r1, #0x18] /* Set WR */
  lsrs          r3, r3, #7      /* GPIO_Pin_1 */
  strh          r3, [r2, #0x18] /* Set D/C */
  bx            lr
//-----------------------------------------------------------------------------
  PUBLIC WriteDataA/* (uint8_t com) */
WriteDataA
  ldr           r1, =0x48000800 /* GPIOC_BASE */
  //ldr           r2, =0x48000400 /* GPIOB_BASE */
  movs          r3, #1          /* GPIO_Pin_8 */
  lsls          r3, r3, #8      /* GPIO_Pin_8 */
  //strh          r3, [r2, #0x28] /* Reset D/C */
  strh          r0, [r1, #0x14] /* write odr */
  //movs          r0, #1
  
  strh          r3, [r1, #0x18] /* Set WR */
 // lsrs          r3, r3, #7      /* GPIO_Pin_1 */
 // strh          r3, [r2, #0x18] /* Set D/C */
  bx            lr
  end