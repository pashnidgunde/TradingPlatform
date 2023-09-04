#include <iostream>
#include <vector>
#include <fstream>
#include <string_view>
#include <type_traits>
#include <bit>
#include <array>

//--------------------------------------------------------------------------------

template<typename T>
struct SwapBytes {
	constexpr T operator()(T val) {
		static_assert(std::is_same_v<T, uint8_t> or
			std::is_same_v<T, uint16_t> or
			std::is_same_v<T, uint32_t> or
			std::is_same_v<T, uint64_t>);

		if constexpr (std::is_same_v<T, uint8_t>)
			return val;

		if constexpr (std::is_same_v<T, uint16_t>)
			return ((((val) >> 8) & 0xff) | (((val) & 0xff) << 8));

		if constexpr (std::is_same_v<T, uint32_t>)
			return ((((val) & 0xff000000) >> 24) |
				(((val) & 0x00ff0000) >> 8) |
				(((val) & 0x0000ff00) << 8) |
				(((val) & 0x000000ff) << 24));

		if constexpr (std::is_same_v<T, uint64_t>)
			return ((((val) & 0xff00000000000000ull) >> 56) |
				(((val) & 0x00ff000000000000ull) >> 40) |
				(((val) & 0x0000ff0000000000ull) >> 24) |
				(((val) & 0x000000ff00000000ull) >> 8) |
				(((val) & 0x00000000ff000000ull) << 8) |
				(((val) & 0x0000000000ff0000ull) << 24) |
				(((val) & 0x000000000000ff00ull) << 40) |
				(((val) & 0x00000000000000ffull) << 56));
	}
};

template<typename T>
constexpr T beToSe(T val) {
	if constexpr (std::endian::native == std::endian::big)
		return val;
	return SwapBytes<T>()(val);
}

template<typename T>
constexpr T seToBe(T val) {
	if constexpr (std::endian::native == std::endian::little)
		return val;
	return SwapBytes<T>()(val);
}

static_assert(SwapBytes<uint8_t>()(0x08) == 0x08);
static_assert(SwapBytes<uint16_t>()(0x0102) == 0x0201);
static_assert(SwapBytes<uint16_t>()(0x01020304) == 0x0403);
static_assert(SwapBytes<uint32_t>()(0x01020304) == 0x04030201);
static_assert(SwapBytes<uint64_t>()(0x0102030405060708) == 0x0807060504030201);

static_assert(beToSe<uint64_t>(0x0102030405060708) == 0x0807060504030201);
static_assert(seToBe<uint64_t>(0x0102030405060708) == 0x0102030405060708);
//static_assert(seToBe<int>(0x0102030405060708) == 0x0102030405060708);  // not supported

//--------------------------------------------------------------------------------

#pragma pack(push, 1)

struct PacketHeader {
	static constexpr size_t SIZE = 6;

	uint16_t sid = 0;
	uint32_t pl = 0;
};
static_assert(sizeof(PacketHeader) == PacketHeader::SIZE);

// Common across all messages
struct MsgHeader {
	static constexpr size_t SIZE_LENGTH_FIELD = 2;
	static constexpr size_t SIZE = 12;

	uint16_t ml;
	char pt;
	char mt;
	uint64_t ts;

};
static_assert(sizeof(MsgHeader) == MsgHeader::SIZE);

struct SystemEvent {
	static constexpr char MT = 'S'; 
	static constexpr size_t SIZE_PAYLOAD = 1;
	static constexpr size_t SIZE = MsgHeader::SIZE + SIZE_PAYLOAD;

	MsgHeader h;
	char ee;
};
static_assert(sizeof(SystemEvent) == SystemEvent::SIZE);

struct AcceptedPayload {
	char ot[14];
	char sd;
	uint32_t s;
	char sym[8];
	uint32_t p;
	uint32_t tif;
	char f[4];
	char d;
	uint64_t orn;
	char cy;
	char is;
	uint32_t mq;
	char ct;
	char os;
};

struct Accepted {
	static constexpr char MT = 'A';
	static constexpr size_t SIZE = 68;

	MsgHeader h;
	AcceptedPayload pl;
};
static_assert(sizeof(Accepted) == Accepted::SIZE);

struct Replaced {
	static constexpr char MT = 'U';
	static constexpr size_t SIZE = 82;

	MsgHeader h;
	AcceptedPayload am;
	char pot[14];
};
static_assert(sizeof(Replaced) == Replaced::SIZE);

struct Executed {
	static constexpr char MT = 'E';
	static constexpr size_t SIZE = 43;

	MsgHeader h;
	char ot[14];
	uint32_t s;
	uint32_t p;
	char lf;
	uint64_t mn;
};
static_assert(sizeof(Executed) == Executed::SIZE);

struct Canceled {
	static constexpr char MT = 'C';
	static constexpr size_t SIZE = 31;

	MsgHeader h;
	char ot[14];
	uint32_t ds;
	char l;	
};
static_assert(sizeof(Canceled) == Canceled::SIZE);

#pragma pack(pop)

//--------------------------------------------------------------------------------

struct Stream {
	struct events {
		uint16_t accepted = 0;
		uint16_t sys_events = 0;
		uint16_t replaced = 0;
		uint16_t canceled = 0;
		uint16_t executed = 0;
		uint16_t executed_shares = 0;
	} e;

	bool hasParked() {
		return size != 0;
	}

	void park(const char* msg, uint8_t ml) {
		memcpy(buff, msg, ml);
		size += ml;
	}

	void complete(const char* msg, uint8_t ml) {
		memcpy(buff + size, msg, ml);
		size += ml;
	}

	std::string_view parked() {
		return std::string_view(buff, size);
	}

	void clearParked() {
		size = 0;
	}

	char buff[128];
	uint8_t size = 0;
	bool valid = false;
};

struct Decorator {
	void decorate(uint8_t id, const Stream& stream) {
		if (!stream.valid) return;

		std::cout << "Stream " << int(id) << '\n';
		std::cout << " Accepted: " << stream.e.accepted << " messages: " << '\n';
		std::cout << " System Event: " << stream.e.sys_events << " messages: " << '\n';
		std::cout << " Replaced: " << stream.e.replaced << " messages" << '\n';
		std::cout << " Canceled: " << stream.e.canceled << " messages" << '\n';
		std::cout << " Exceuted: " << stream.e.executed << " messages: " << stream.e.executed_shares << " executed shares" << '\n';
		std::cout << '\n';
	}
};

struct OutchParser {

	template<typename T>
	static constexpr bool isComplete(const T* msg, const uint16_t pl) {
		auto st = sizeof(T);
		return pl == sizeof(T);
	}

	template<typename T>
	static const T* as(const char* offset) {
		return reinterpret_cast<const T*>(offset);
	}

	template<typename T>
	struct Decoder {
		void operator()(const T* msg, Stream& stream) {
			throw std::runtime_error("Unsupported Msg");
		}
	};

	template<>
	struct Decoder<SystemEvent> {
		void operator()(const SystemEvent* msg, Stream& stream) {
			stream.e.sys_events++;
		}
	};

	template<>
	struct Decoder<Accepted> {
		void operator()(const Accepted* msg, Stream& stream) {
			stream.e.accepted++;
		}
	};

	template<>
	struct Decoder<Replaced> {
		void operator()(const Replaced* msg, Stream& stream) {
			stream.e.replaced++;
		}
	};

	template<>
	struct Decoder<Executed> {
		void operator()(const Executed* msg, Stream& stream) {
			stream.e.executed++;
			stream.e.executed_shares += beToSe(msg->s);
		}
	};

	template<>
	struct Decoder<Canceled> {
		void operator()(const Canceled* msg, Stream& stream) {
			stream.e.canceled++;
		}
	};

	template<typename T>
	void try_decode(const char* offset, uint8_t pl, Stream& stream)
	{
		auto e = as<T>(offset);
		if (isComplete<T>(e, pl)) {
			try {
				Decoder<T>()(e, stream);
			}
			catch (std::exception& e) {
				std::cout << "Exception : " << e.what();
			}
			return;
		}

		stream.park(offset, pl);
	}

	void decode(const char* offset, uint8_t pl, Stream& stream)
	{
		char mt = msgType(offset);
		switch (mt)
		{
		case SystemEvent::MT:
			try_decode<SystemEvent>(offset, pl, stream);
			break;
		case Accepted::MT:
			try_decode<Accepted>(offset, pl, stream);
			break;
		case Replaced::MT:
			try_decode<Replaced>(offset, pl, stream);
			break;
		case Executed::MT:
			try_decode<Executed>(offset, pl, stream);
			break;
		case Canceled::MT:
			try_decode<Canceled>(offset, pl, stream);
			break;
		default:
			break;
		}

		stream.valid = true;
	}


	void decode(uint8_t sid, uint8_t pl, const char* offset) {
		auto& stream = _streams[sid];
		if (stream.hasParked()) {
			stream.complete(offset, pl);
			auto parked = stream.parked();
			decode(parked.data(), parked.size(), stream);
			stream.clearParked();
			return;
		}

		decode(offset, pl, stream);
	}

	OutchParser() = delete;
	OutchParser(const std::string_view& b) : _b(b) {}
	OutchParser(const char* buffer, size_t len) : _b(buffer, len) {}

	const char msgType(const char* msgoffset) {
		auto msg_type_offset = msgoffset + sizeof(MsgHeader::ml) + sizeof(MsgHeader::pt);
		return *reinterpret_cast<const char*>(msg_type_offset);
	}

	void parse() {
		auto sid = 0;
		auto pl = 0;
		size_t offset = 0;
		const char* begin = _b.data();
		const auto size = _b.size();
		const char* next = nullptr;
		while (offset < size) {
			next = begin + offset;
			auto ph = as<PacketHeader>(next);

			sid = beToSe(ph->sid);
			pl = beToSe(ph->pl);

			decode(sid, pl, next + PacketHeader::SIZE);

			offset += PacketHeader::SIZE + pl;
		}
	}

	const auto& results() {
		return _streams;
	}

	std::string_view _b;
	std::array<Stream, std::numeric_limits<uint8_t>::max()> _streams;
};


struct FileReader {
	explicit FileReader(const std::string& filename) {
		if (!try_read(filename)) {
			throw std::runtime_error("Couldn't read the file");
		}
	}

	bool try_read(const std::string& file_name) {
		std::fstream fs;
		fs.open(file_name, std::ios::in | std::ios::binary);
		if (!fs.is_open()) return false;

		fs.seekg(0, std::ios::end);
		_sz = fs.tellg();
		if (_sz == 0) return false;
		fs.seekg(0, std::ios::beg);

		_data = std::make_unique<char[]>(_sz);
		
		fs.read(_data.get(), _sz);
		fs.close();
		return true;
	}
	
	std::string_view data() const {
		return std::string_view(_data.get(), _sz);
	}

	std::unique_ptr<char[]> _data;
	size_t _sz = 0;

};



int main()
{
	try {
		FileReader fr("D:/OUCHLMM2.incoming.packets");
		OutchParser op(fr.data());
		op.parse();

		Decorator d;
		auto& results = op.results();
		for (int8_t sid = 0; sid < results.size(); sid++) {
			d.decorate(sid, results[sid]);
		}
	}
	catch(const std::exception& e) {
		std::cout << "Exception : " << e.what() << std::endl;
	}

	return 0;
}
