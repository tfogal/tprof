/*
 * This file is part of libtprof.
 *
 * libtprof is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libtprof is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libtprof.  If not, see <http://www.gnu.org/licenses/>.
 */
/** Basic test program for libtprof. */
#include <stdio.h>

void function();
int
main()
{
    puts("yay main!");
    function();
    puts("back into main.");
    return 0;
}
void
function()
{
    puts("function!");
}
