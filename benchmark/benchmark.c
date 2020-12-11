//******************************************************************************
// system includes
//******************************************************************************

#include <stdio.h>
#include <unistd.h>     // write



//******************************************************************************
// local includes
//******************************************************************************

#include "benchmark.h"

address addrs[ADDRESS_SIZE] = {
{"716 Glenlake Ave. "},
{"Powhatan, VA 23139"},
{"9628 Middle River Street "},
{"Grand Forks, ND 58201"},
{"8512 S. Arnold Drive "},
{"Scarsdale, NY 10583"},
{"7521 Walnut Ave."}, 
{"Port Jefferson Station, NY 11776"},
{"26 Tanglewood Dr. "},
{"Tallahassee, FL 32303"},
{"331 West Lexington Street "},
{"Oshkosh, WI 54901"},
{"59 Cherry Road "},
{"Massapequa, NY 11758"},
{"7930 Nichols St."}, 
{"Mableton, GA 30126"},
{"609 Delaware Road "},
{"Unit 905 "},
{"Nicholasville, KY 40356"},
{"35 Bowman Drive "},
{"Apt 2 "},
{"Livingston, NJ 07039"},
{"21 Sycamore Lane "},
{"Carpentersville, IL 60110"},
{"84 Forest Dr. "},
{"Windermere, FL 34786"},
{"8044 Goldfield St. "},
{"Newtown, PA 18940"},
{"248 S. East St. "},
{"Huntington, NY 11743"},
{"85 Arnold Street "},
{"Apt E "},
{"Central Islip, NY 11722"},
{"7468 Maiden St. "},
{"Thomasville, NC 27360"},
{"51 Devon Avenue "},
{"Palm Harbor, FL 34683"},
{"238 N. Orange Dr. "},
{"Minot, ND 58701"},
{"39 Big Rock Cove Lane "},
{"Plainfield, NJ 07060"},
{"99 Maple Road "},
{"Pearl, MS 39208"},
{"147 East Del Monte Rd. "},
{"Attleboro, MA 02703"},
{"667 Saxton Road "},
{"Sumter, SC 29150"},
{"8195 E. Roehampton St. "},
{"Lincolnton, NC 28092"},
{"34 E. Oak Dr. "},
{"Blackwood, NJ 08012"},
{"287 Vernon Ave. "},
{"Reisterstown, MD 21136"},
{"957 Temple Avenue "},
{"Bowie, MD 20715"},
{"766 Buckingham St. "},
{"Hamilton, OH 45011"},
{"907 Pin Oak Dr. "},
{"Rolla, MO 65401"},
{"8617 Beech Street "},
{"Jacksonville, NC 28540"},
{"39 South Tower St. "},
{"Grove City, OH 43123"},
{"54 Pumpkin Hill Court "},
{"Elizabethton, TN 37643"},
{"47 Fieldstone St. "},
{"Wilmette, IL 60091"},
{"424 Crescent Drive "},
{"Parkersburg, WV 26101"},
{"7672 N. Stonybrook Ave. "},
{"Port Huron, MI 48060"},
{"40 Morris Lane "},
{"Flushing, NY 11354"},
{"19 10th Ave. "},
{"Burbank, IL 60459"},
{"7 Hartford St. "},
{"Norristown, PA 19401"},
{"65 Manhattan Street "},
{"Coram, NY 11727"},
{"46 Arcadia Street "},
{"Clarksburg, WV 26301"},
{"533 Glen Ridge St. "},
{"Prattville, AL 36067"},
{"64 Blue Spring St. "},
{"Neenah, WI 54956,"},
{"3 Mulberry St. "},
{"Elk Grove Village, IL 60007"},
{"44 Cedar Swamp Ave. "},
{"Loveland, OH 45140"},
{"86 Chapel Drive "},
{"West Roxbury, MA 02132"},
{"8154 Briarwood Street "},
{"Tuckerton, NJ 08087"},
{"161 Hudson St."},
{"Newton, NJ 07860"},
{"93 Cobblestone Dr. "},
{"Coachella, CA 92236"},
{"8480 Pawnee Ave."}, 
{"Trussville, AL 35173"},
{"2 N. Big Rock Cove Ave. "},
{"Moses Lake, WA 98837"},
{"7727 Rockcrest St. "},
{"Onalaska, WI 54650"},
{"7 W. Euclid Street "},
{"Cary, NC 27511"},
{"8537 Market Rd."},
{"Gibsonia, PA 15044"},
{"93 East Beechwood Street "},
{"Gallatin, TN 37066"},
{"79 Fairway Rd."},
{"Evanston, IL 60201"},
{"819 Marlborough Road "},
{"East Hartford, CT 06118"},
{"52 South Sleepy Hollow Street "},
{"Fort Lauderdale, FL 33308"},
{"88 Rock Maple St. "},
{"State College, PA 16801"},
{"201 Bear Hill St. "},
{"Elizabeth, NJ 07202"},
{"9198 Griffin Ave. "},
{"Southgate, MI 48195"},
{"14 Lakewood Lane"},
{"Mchenry, IL 60050"}};



//******************************************************************************
// interface operations
//******************************************************************************

void print_address(uint64_t thread_index, int key, address *addr) {
  if (!addr) {
      printf("thread: %u key: %d: address passed is NULL\n", thread_index, key);
      return;
  }
  /* We use sprintf + write instead of printf. Helgrind reports data-races for the latter */
  char buffer[75];
  int stdout = 1;
  int char_count = sprintf(buffer, "thread: %u key: %d: address:  %s\n", thread_index, key, addr->street_name);
  write(stdout, buffer, char_count);
}