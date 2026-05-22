.section .text

# these are the mangled names for each function, which are static methods.
.globl _ZN6Physic4initEjss
.globl _ZN6Physic12compute_pairEPjS0_
.globl _ZN6Physic10get_linearEj
.globl _ZN6Physic14euclidean_normEPjPt
.globl _ZN6Physic6deinitEv

# performs packed euclidean norm of vectors
_ZN6Physic14euclidean_normEPjPt: # mm* velocity vectors, u16* norm vector
  movl 4(%esp), %edx # load ptr into edx

  movq (%edx),  %mm2 # load vectors into mm2

  pmaddwd %mm2, %mm2 # get | x_1^2 + y_1^2 | x_0^2 + y_0^2 |
  cvtpi2ps %mm2, %xmm2 # convert them both into floats to do the sqrt. has to be in xmm for that
  sqrtps %xmm2, %xmm2 # load the square roots into xmm2
  cvtps2pi %xmm2, %mm2 # integer, cast back down to mm2 (and round, rather than truncate with cvtt)

  # maximum possible is now 0xb505, which is cool because the norm is always unsigned
  pshufw $0b01011000, %mm2, %mm2 # shuffle, because pack with unsigned saturation is too much to ask...
  # packed into | 0 | 0 | m_1 | m_0 |

  movl 8(%esp), %ecx # load output addr into ecx
  movd %mm2, (%ecx) # load to output address

  ret

_ZN6Physic4initEjss: # u16 tick, i16 gravity, i16 wind
  movw 4(%esp), %ax # load tick
  movl $8, %ecx # repeat it eight times (128-bits)

  pushl %edi
  movl $__physic_data_tick, %edi
  rep stosw # write tick to every word
  popl %edi

  # create acceleration word. e.g. | 0 | -9.81 | 0 | 0 |
  /*
    what we want to do:
      movw 8(%esp), (acceleration + 4) # y-axis acceleration, needs to be low 16
      movw 12(%esp), (acceleration + 6) # x-axis acceleration, needs to be high 16
    unfortunately not possible, as cant move between two addresses. so we do simd magic
  */

  # load both parameters. this yields | 0 | a_x | 0 | a_y |
  # | 0 | a_x | 0 | a_y | => | a_x | a_y | a_x | a_y |.
  # we move this to higher using movd, as pshuf can't output to address
  # packsswb isnt used as we then can't do memory magic
  pshufw $0b10001000, 8(%esp), %mm6
  movq %mm6, (__physic_data_acceleration)

  ret

# each object is represented by a packet of | v_x | v_y | x | y |
# where each cell is a 16-bit fixed point (9.7) number, representing position.
# 8000 and 7fff are the edges of the screen, so we can use signed saturation.
# must be signed as we are decreasing.
# v_x, v_y is the velocity vector.
# x, y is the position vector
_ZN6Physic12compute_pairEPjS0_: # mm* position vectors, mm* velocity vectors
  movq (__physic_data_acceleration), %mm5 # mm5 <- a
  pmullw (__physic_data_tick), %mm5 # mm5 = a * t. needed for both equations

  # we use xmm stuff so as to do both at once
  movq2dq %mm5, %xmm1 # xmm1 <- at
  psraw $1, %xmm1 # xmm1 = .5at. needed for s = ut + .5at^2

  pushl %esi
  pushl %edi

  movl 12(%esp), %esi # position vector ptr. 12 because we pushed esi and edi
  movl 16(%esp), %edi # velocity vector ptr. 16 because ditto
  # load velocity vector into low bytes of xmm
  movq (%edi), %xmm0 # load velocity vector into low bytes of xmm

  # calculating s = ut + .5at^2.
  # what we do is we get a vector of the form | u_x | .5at_x | u_y | .5at_y | ... etc.
  # (hence low unpack into xmm: we do both mms here, so xmm is used)
  # which we then pmaddwd with t, to get | (ut+.5at^2)_x | (ut+.5at^2)_y |.
  punpcklwd %xmm0, %xmm1 # unpack low into vector
  pmaddwd (__physic_data_tick), %xmm1 # pmaddwd with t
  packssdw %xmm1, %xmm1 # squash back down into 16bit words. we use itself cause we don't care about high 32
  movdq2q %xmm1, %mm4 # mm4 = Ds packet

  paddsw (%edi), %mm5 # add Dv packet. .5at is already saved, so we don't care
  paddsw (%esi), %mm4 # add Ds packet.
  # save them
  movq %mm5, (%edi)
  movq %mm4, (%esi)

  call __physic_bounce_post_save

  popl %edi # undo push
  popl %esi
  ret # eax is now linear coordinate

_ZN6Physic10get_linearEj: # get_linear(u32 pos)
  # now, we get new position offsets to our draw function
  movd 4(%esp), %mm0
  pxor (__physic_data_invert_mask), %mm0 # our mask.
        # this mask inverts top bits, to shift signed into unsigned
        # (as we've been saturating signedly), and also inverts again the whole of y axis
        # as the origin is in top-left, not top-right as we might expect

  # unsigned multiply, becuase we've now normalised them to unsigned
  pmulhuw (__physic_data_bounds), %mm0 # normalise our 0x0000-0xffff to 0-80 and 0-25.
    # we do this by multiplying them by the desired width, and only take the high 16 of each.

  movd %mm0, %edx  # edx is now valid coordinates (x, y), with x taking up high 16

  movb $80, %al    # multiply y by 80 to get linear coordinate, and put it in al
  mulb %dl

  shrl $16, %edx # get high 16 (x coordinate) from packet
  addw %dx, %ax # add x coordinate

  ret

# check for bouncing. if it is at the edges (e.g. y = 0x8000), bounce
__physic_bounce_post_save:
  # we want to check both 0x7fff and 0x8000. thankfully, our invert mask from earlier does the trick...
  # both are used as it inverts it later
  movq (__physic_data_edge_check_1), %mm4 # bounce
  pcmpeqw (%esi), %mm4 # mm4 now stores if position packets are equal to first bounce

  movq (__physic_data_edge_check_2), %mm5 # bounce
  pcmpeqw (%esi), %mm5 # mm5 now stores if ditto for second bounce

  por %mm4, %mm5 # mm5 now stores final mask.

  movq (__physic_data_restitution_coeffs), %mm4

  /*
    what we want to do:
      # special multiply, that does (((a * b) >> 14) + 1) >> 1 \approx (a*b) >> 15.
      # best way to negate and multiply.
      pmulhrsw (%edi), %mm4 # multiply velocity packet by scale factors.
                            # mm4 is now the scaled "bounced" velocity.
    not possible, as pmulhrsw is ssse3 only.
    this is bad, because we can only now store coefficients down to -0.5 in 16 bits.
    we thus keep our coefficients equal (halving them), then shl by 1 to account for inaccuracy
  */
  pmulhw (%edi), %mm4 # multiply velocity packet by scale factors.
  psllw $1, %mm4      # shift left to account for inaccuracy
                      # mm4 is now the scaled "bounced" velocity.

  # note: this takes edi as implicit parameter. good thing it's already set...
  maskmovq %mm5, %mm4 # selectively write the bounced data when mask says it's equal.

  ret

_ZN6Physic6deinitEv:
  emms
  ret

.section .bss
.align 16 # xmm pmaddwd chokes if it's not aligned (my guess is it throws out the bottom few bits)
  __physic_data_tick: # xmm word, as it used for pmaddwd
    .octa 0

  __physic_data_acceleration: # storing the acceleration word.
    .long 0

.section .rodata
  __physic_data_bounds: # dimensions of the screen, in the positiong cells: | 80 | 25 | 0 | 0 |
    .word 25 # height
    .word 80 # width
    .word 0 # not needed, as get_linear operates one at a time. TODO maybe?
    .word 0

  __physic_data_edge_check_1: # edge checking in bounce code. like this so that we can reuse the start of invert_mask
    .word 0x8000

  __physic_data_edge_check_2:
  __physic_data_invert_mask: # used for xoring to invert the y axis because of moving origin
               # also flips the top bit, which shifts our signed numbers into unsigned
    .word 0x7fff
    .word 0x8000
    .word 0x7fff
    .word 0x8000
    # .word 0
    # .word 0 # these are repeated next, so as to not repeat myself i've commented them out

  __physic_data_restitution_coeffs: # create | -0.6 | -0.8 | -0.6 | -0.8 |, which are the negatives of the coefficients of restitution. walls absorb more kinetic energy than floor/ceiling, so absorb 40% rather than 20%
    .word 0xb333 # -0.6
    .word 0x999a # -0.8
    .word 0xb333 # -0.6
    .word 0x999a # -0.8
