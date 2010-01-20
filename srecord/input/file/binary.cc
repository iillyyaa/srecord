//
//      srecord - manipulate eprom load files
//      Copyright (C) 1998-2000, 2002, 2003, 2006-2008, 2010 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <srecord/input/file/binary.h>
#include <srecord/record.h>


srec_input_file_binary::~srec_input_file_binary()
{
}


srec_input_file_binary::srec_input_file_binary(const std::string &a_file_name) :
    srec_input_file(a_file_name),
    address(0)
{
}


srec_input::pointer
srec_input_file_binary::create(const std::string &a_file_name)
{
    return pointer(new srec_input_file_binary(a_file_name));
}


bool
srec_input_file_binary::read(srec_record &record)
{
#ifdef HAVE_SPARSE_LSEEK
    //
    // On Solaris, they have a couple of new lseek whences in order to
    // navigate around holes in the file data.
    //
    //     fpathconf(fd, _PC_MIN_HOLE_SIZE);
    //     lseek(fd, address, SEEK_DATA);
    //     lseek(fd, address, SEEK_HOLE);
    //
    // Using these, it would be possible to actually report file holes
    // via the srecord API.
    //
#endif

    int c = get_char();
    if (c < 0)
        return false;
    int length = 0;
    unsigned char data[srec_record::max_data_length];
    for (;;)
    {
        data[length++] = c;
        if (length >= (int)sizeof(data))
            break;
        c = get_char();
        if (c < 0)
            break;
    }
    record = srec_record(srec_record::type_data, address, data, length);
    address += length;
    return true;
}


const char *
srec_input_file_binary::mode()
    const
{
    return "rb";
}


const char *
srec_input_file_binary::get_file_format_name()
    const
{
    return "Binary";
}