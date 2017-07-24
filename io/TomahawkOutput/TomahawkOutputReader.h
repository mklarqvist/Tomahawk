#ifndef TOMAHAWKOUTPUTREADER_H_
#define TOMAHAWKOUTPUTREADER_H_

#include <vector>
#include <fstream>
#include <string>
#include <stddef.h>
#include <regex>

#include "../../TypeDefinitions.h"
#include "../../tomahawk/MagicConstants.h"
#include "TomahawkOutputEntry.h"
#include "../../io/PackedEntryReader.h"
#include "TomahawkOutputFilterController.h"
#include "../../io/BasicBuffer.h"
#include "../../io/GZController.h"
#include "../../io/BasicWriters.h"
#include "../../io/totempole/TotempoleMagic.h"
#include "../../io/TGZFHeader.h"


namespace Tomahawk {
namespace IO {
// Todo: TomahawkOutputIndexReader

class TomahawkOutputReader {
	typedef TomahawkOutputEntry entry_type;
	typedef TomahawkOutputFilterController filter_type;
	typedef PackedEntryReader<entry_type, sizeof(entry_type)> reader_type;

public:
	TomahawkOutputReader();
	~TomahawkOutputReader(){ }

	// Streaming functions
	bool getBlock(const U32 blockID);
	bool getBlock(std::vector< std::pair<U32, U32> >& pairs);
	bool getBlocks(void);

	bool Open(const std::string input){
		this->stream.open(input, std::ios::binary | std::ios::in | std::ios::ate);
		if(!this->stream.good()){
			std::cerr << "failed to open file " << input << std::endl;
			return false;
		}

		this->filesize = this->stream.tellg();
		this->stream.seekg(0);

		if(!this->stream.good()){
			std::cerr << "bad stream" << std::endl;
			return false;
		}

		this->stream >> this->header;
		if(!this->header.validate(Tomahawk::Constants::WRITE_HEADER_LD_MAGIC)){
			std::cerr << "failed to validate header" << std::endl;
			return false;
		}

		if(!this->ParseHeader()){
			std::cerr << "failed to parse header" << std::endl;
			return false;
		}

		return true;
	}

	bool ParseHeader(void){
		Totempole::TotempoleContigBase* base = new Totempole::TotempoleContigBase[this->header.n_contig];
		for(U32 i = 0; i < this->header.n_contig; ++i){
			this->stream >> base[i];
			std::cerr << base[i] << std::endl;
			// Todo: Put data into hash table for lookups
		}

		delete [] base;

		return true;
	}

	bool nextBlock(void){
		if(!this->stream.good()){
			std::cerr << "stream died" << std::endl;
			return false;
		}

		if(this->stream.tellg() == this->filesize){
			std::cerr << "eof" << std::endl;
			return false;
		}

		buffer.resize(sizeof(TGZFHeader));
		this->stream.read(&buffer.data[0],  Constants::TGZF_BLOCK_HEADER_LENGTH);
		const TGZFHeader* h = reinterpret_cast<const TGZFHeader*>(&buffer.data[0]);
		buffer.pointer = Constants::TGZF_BLOCK_HEADER_LENGTH;
		if(!h->Validate()){
			std::cerr << "failed to validate" << std::endl;
			return false;
		}

		buffer.resize(h->BSIZE);
		// Recast because if resized then pointer address is incorrect
		h = reinterpret_cast<const TGZFHeader*>(&buffer.data[0]);

		this->stream.read(&buffer.data[Constants::TGZF_BLOCK_HEADER_LENGTH], h->BSIZE - Constants::TGZF_BLOCK_HEADER_LENGTH);
		if(!this->stream.good()){
			std::cerr << "truncated file" << std::endl;
			return false;
		}

		buffer.pointer = h->BSIZE;
		const U32* outsize = reinterpret_cast<const U32*>(&buffer[buffer.pointer -  sizeof(U32)]);
		//const U32* crc = reinterpret_cast<const U32*>(&buffer[buffer.pointer -  sizeof(U32) - sizeof(U32)]);
		//std::cerr << *outsize << '\t' << *crc << std::endl;
		output_buffer.resize(*outsize);
		this->output_buffer.reset();

		if(!this->gzip_controller.Inflate(buffer, output_buffer)){
			std::cerr << "failed inflate" << std::endl;
			return false;
		}

		if(this->output_buffer.size() == 0){
			std::cerr << "empty data" << std::endl;
			return false;
		}

		std::cerr << output_buffer.pointer << std::endl;
		this->buffer.reset();

		// Reset iterator position and size
		this->position = 0;
		this->size = this->output_buffer.size() / sizeof(TomahawkOutputEntry);

		// Validity check
		if(this->output_buffer.size() % sizeof(TomahawkOutputEntry) != 0){
			std::cerr << "data is corrupted" << std::endl;
			return false;
		}

		return true;
	}

	const TomahawkOutputEntry* operator[](const U32 p) const{ return(reinterpret_cast<TomahawkOutputEntry*>(&this->output_buffer.data[sizeof(TomahawkOutputEntry)*p])); }

	bool nextVariant(const TomahawkOutputEntry*& entry){
		if(this->position == this->size){
			if(!this->nextBlock())
				return false;
		}

		entry = (*this)[this->position];
		++this->position;

		return true;
	}

	// Other
	bool view(const std::string& filename);
	bool index(const std::string& filename);
	bool summary(const std::string& input);

	// Read entire file into memory
	filter_type& getFilter(void){ return this->filter; }

private:
	bool __viewOnly(void);
	bool __viewFilter(void);

public:
	U64 samples; 	// has to match header
	float version;	// has to match header
	U64 filesize;	// input file size

	U32 position;
	U32 size;

	std::ifstream stream; // reader stream
	TomahawkOutputHeader<Tomahawk::Constants::WRITE_HEADER_LD_MAGIC_LENGTH> header; // header

	IO::BasicBuffer buffer; // internal buffer
	IO::BasicBuffer output_buffer; // internal buffer
	IO::GZController gzip_controller; // TGZF controller
	filter_type filter;	// filter parameters
	IO::GenericWriterInterace* writer; // writer interface
	// Todo: PackedEntryIterator taking as input char* and length or IO::BasicBuffer

	//temp
	reader_type reader;
};

}
} /* namespace Tomahawk */

#endif /* TOMAHAWKOUTPUTREADER_H_ */
