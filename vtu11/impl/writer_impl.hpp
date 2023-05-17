//          __        ____ ____
// ___  ___/  |_ __ _/_   /_   |
// \  \/ /\   __\  |  \   ||   |
//  \   /  |  | |  |  /   ||   |
//   \_/   |__| |____/|___||___|
//
//  License: BSD License ; see LICENSE
//

#ifndef VTU11_WRITER_IMPL_HPP
#define VTU11_WRITER_IMPL_HPP

#include "vtu11/inc/utilities.hpp"

#include <fstream>
#include <type_traits>

namespace vtu11
{
namespace detail
{

template<typename T> inline
void writeNumber( char (&)[64], T )
{
    VTU11_THROW( "Invalid data type." );
}

#define VTU11_WRITE_NUMBER_SPECIALIZATION( string, type )     \
template<> inline                                             \
void writeNumber<type>( char (&buffer)[64], type value )      \
{                                                             \
    std::snprintf( buffer, sizeof( buffer ), string, value ); \
}

VTU11_WRITE_NUMBER_SPECIALIZATION( VTU11_ASCII_FLOATING_POINT_FORMAT, double )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%lld", long long int )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%ld" , long int )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%d"  , int )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%hd" , short )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%hhd", char )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%llu", unsigned long long int )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%ld" , unsigned long int )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%d"  , unsigned int )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%hd" , unsigned short )
VTU11_WRITE_NUMBER_SPECIALIZATION( "%hhd", unsigned char )

} // namespace detail

template<typename TIteratorType>
inline void AsciiWriter::writeData( std::ostream& output,
                                    TIteratorType begin,
                                    const size_t n )
{
    char buffer[64];

    for(size_t i = 0; i < n; ++i)
    {
        if constexpr(std::is_same_v<std::remove_pointer_t<TIteratorType>, const signed char>)
        {
            output << static_cast<int>( *(begin++) ) << " ";

        }
        else
        {
          detail::writeNumber( buffer, *(begin++) );

          output << buffer << " ";
        }
    }

    output << "\n";
}

inline void AsciiWriter::writeAppended( std::ostream& )
{

}

inline void AsciiWriter::addHeaderAttributes( StringStringMap& )
{
}

inline void AsciiWriter::addDataAttributes( StringStringMap& attributes )
{
  attributes["format"] = "ascii";
}

inline StringStringMap AsciiWriter::appendedAttributes( )
{
  return { };
}

// ----------------------------------------------------------------

template<typename TIteratorType>
inline void Base64BinaryWriter::writeData( std::ostream& output,
                                           TIteratorType begin,
                                           const size_t n )
{
  HeaderType numberOfBytes = n * sizeof( decltype( *begin ) );

  Base64EncodedOutput base64output;
  base64output.writeOutputData( output, &numberOfBytes, 1 );
  base64output.writeOutputData( output, begin, n );
  base64output.closeOutputData( output );

  output << "\n";
}

inline void Base64BinaryWriter::writeAppended( std::ostream& )
{

}

inline void Base64BinaryWriter::addHeaderAttributes( StringStringMap& attributes )
{
  attributes["header_type"] = dataTypeString<HeaderType>( );
}

inline void Base64BinaryWriter::addDataAttributes( StringStringMap& attributes )
{
  attributes["format"] = "binary";
}

inline StringStringMap Base64BinaryWriter::appendedAttributes( )
{
  return { };
}

// ----------------------------------------------------------------

template<typename TIteratorType>
inline void Base64BinaryAppendedWriter::writeData( std::ostream& output,
                                                   TIteratorType begin,
                                                   const size_t n )
{
  HeaderType rawBytes = n * sizeof( decltype(*begin) );

  this->appendedData.emplace_back( reinterpret_cast<const char*>( begin ), rawBytes );

  offset += encodedNumberOfBytes( rawBytes + sizeof( HeaderType ) );
}

inline void Base64BinaryAppendedWriter::writeAppended( std::ostream& output )
{
  for( auto dataSet : appendedData )
  {
    Base64EncodedOutput base64output;
    base64output.writeOutputData( output, &dataSet.second, 1 );
    base64output.writeOutputData( output, dataSet.first, dataSet.second );
    base64output.closeOutputData( output );
  }

  output << "\n";
}

inline void Base64BinaryAppendedWriter::addHeaderAttributes( StringStringMap& attributes )
{
  attributes["header_type"] = dataTypeString<HeaderType>( );
}

inline void Base64BinaryAppendedWriter::addDataAttributes( StringStringMap& attributes )
{
  attributes["format"] = "appended";
  attributes["offset"] = std::to_string( offset );
}

inline StringStringMap Base64BinaryAppendedWriter::appendedAttributes( )
{
  return { { "encoding", "base64" } };
}

// ----------------------------------------------------------------

template<typename TIteratorType>
inline void RawBinaryAppendedWriter::writeData( std::ostream& output,
                                                TIteratorType begin,
                                                const size_t n )
{
  HeaderType rawBytes = n * sizeof( decltype(*begin) );

  appendedData.emplace_back( reinterpret_cast<const char*>( begin ), rawBytes );

  offset += sizeof( HeaderType ) + rawBytes;
}

inline void RawBinaryAppendedWriter::writeAppended( std::ostream& output )
{
  for( auto dataSet : appendedData )
  {
    const char* headerBegin = reinterpret_cast<const char*>( &dataSet.second );

    for( const char* ptr = headerBegin; ptr < headerBegin + sizeof( HeaderType ); ++ptr )
    {
      output << *ptr;
    }

    for( const char* ptr = dataSet.first; ptr < dataSet.first + dataSet.second; ++ptr )
    {
      output << *ptr;
    }
  }

  output << "\n";
}

inline void RawBinaryAppendedWriter::addHeaderAttributes( StringStringMap& attributes )
{
  attributes["header_type"] = dataTypeString<HeaderType>( );
}

inline void RawBinaryAppendedWriter::addDataAttributes( StringStringMap& attributes )
{
  attributes["format"] = "appended";
  attributes["offset"] = std::to_string( offset );
}

inline StringStringMap RawBinaryAppendedWriter::appendedAttributes( )
{
  return { { "encoding", "raw" } };
}

} // namespace vtu11

#endif // VTU11_WRITER_IMPL_HPP
