//
//      srecord - manipulate eprom load files
//      Copyright (C) 2004, 2006-2008, 2010 Peter Miller
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

#include <cstring>

#include <srecord/output/file/stewie.h>
#include <srecord/record.h>


srec_output_file_stewie::~srec_output_file_stewie()
{
    if (enable_footer_flag)
    {
        //
        // Even though struct Motorola compatibility would seem to
        // indicate the S7, S8 or S9 could terminate the file, only
        // S8 seems to work.
        //
        write_inner(8, 0, 0, 0, 0);
    }
}


srec_output_file_stewie::srec_output_file_stewie(
        const std::string &a_file_name) :
    srec_output_file(a_file_name),
    data_count(0),
    address_length(2),
    preferred_block_size(128)
{
    if (line_termination == line_termination_native)
        line_termination = line_termination_binary;
}


srec_output::pointer
srec_output_file_stewie::create(const std::string &a_file_name)
{
    return pointer(new srec_output_file_stewie(a_file_name));
}


void
srec_output_file_stewie::put_byte(unsigned char n)
{
    put_char(n);
    checksum_add(n);
}


void
srec_output_file_stewie::write_inner(int tag, unsigned long address,
    int address_nbytes, const void *data, int data_nbytes)
{
    //
    // Make sure the record is not too long.
    //
    if (address_nbytes + data_nbytes > 254)
    {
        fatal_error
        (
            "data length (%d+%d) too long",
            address_nbytes,
            data_nbytes
        );
    }

    //
    // Assemble the data for this record.
    //
    unsigned char buffer[256];
    int line_length = address_nbytes + data_nbytes + 1;
    buffer[0] = line_length;
    srec_record::encode_big_endian(buffer + 1, address, address_nbytes);
    if (data_nbytes)
        memcpy(buffer + 1 + address_nbytes, data, data_nbytes);

    //
    // Emit the record as binary data.
    //
    put_char('S');
    put_nibble(tag);
    switch (tag)
    {
    case 0:
        put_char('0');
        put_char('3');
        break;

    case 7:
    case 8:
    case 9:
        break;

    default:
        checksum_reset();
        for (int j = 0; j < line_length; ++j)
            put_byte(buffer[j]);
        put_byte(~checksum_get());
        break;
    }
}


void
srec_output_file_stewie::write(const srec_record &record)
{
    switch (record.get_type())
    {
    case srec_record::type_header:
        if (enable_header_flag)
        {
            //
            // Even though it starts with S0, the header record has a
            // fixed format, so we don't bother passing it any data.
            //
            write_inner(0, 0, 0, 0, 0);
        }
        break;

    case srec_record::type_data:
        if
        (
            record.get_address() < (1UL << 16)
        &&
            address_length <= 2
        )
        {
            write_inner
            (
                1,
                record.get_address(),
                2,
                record.get_data(),
                record.get_length()
            );
        }
        else if
        (
            record.get_address() < (1UL << 24)
        &&
            address_length <= 3
        )
        {
            write_inner
            (
                2,
                record.get_address(),
                3,
                record.get_data(),
                record.get_length()
            );
        }
        else
        {
            write_inner
            (
                3,
                record.get_address(),
                4,
                record.get_data(),
                record.get_length()
            );
        }
        ++data_count;
        break;

    case srec_record::type_data_count:
    case srec_record::type_execution_start_address:
        // ignore
        break;

    case srec_record::type_unknown:
        fatal_error("can't write unknown record type");
    }
}


void
srec_output_file_stewie::line_length_set(int n)
{
    if (n < 1)
        n = 1;
    else if (n > 250)
        n = 250;
    preferred_block_size = n;
}


void
srec_output_file_stewie::address_length_set(int n)
{
    if (n < 2)
        n = 2;
    else if (n > 4)
        n = 4;
    address_length = n;
}


int
srec_output_file_stewie::preferred_block_size_get()
    const
{
    return preferred_block_size;
}


const char *
srec_output_file_stewie::format_name()
    const
{
    return "Stewie";
}