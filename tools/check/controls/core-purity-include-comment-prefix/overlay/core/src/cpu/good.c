/* SPDX-License-Identifier: MIT */
/* Control core-purity-include-comment-prefix: a stand-in core whose host includes follow a block
 * comment on the same line (review WR-04). A comment counts as whitespace, so both lines are
 * directives: one after a whole comment, one after the end of a comment opened on the line above. */
#include "common/ivs_prelude.h"
/* host */ #include <unistd.h>
/* a comment that ends
   here */ #include <stdlib.h>

int ivs_good_value(void);

int ivs_good_value(void)
{
  return 1;
}
