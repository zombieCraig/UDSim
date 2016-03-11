using System;
using System.Collections.Generic;
using Peach;
using System.IO;
using System.Net;
using System.Runtime.InteropServices;
using NLog;
using Mono.Unix;
using Mono.Unix.Native;
using Peach.Core.IO;

namespace Peach.Core.Publishers
{
	[Publisher("CAN", true)]
	[Parameter("Interface", typeof(string), "Name of interface to bind to")]
	[Parameter("Baud", typeof(int), "CAN Bus baud rate (default 500000)", "500000")]
        [Parameter("MinMTU", typeof(uint), "Minimum allowable MTU property value", "16")]
        [Parameter("MaxMTU", typeof(uint), "Maximum allowable MTU property value", "16")]
 	public class CanPublisher : Publisher
	{
#region P/Invokes

		const int AF_CAN     = 29;
		const int SOCK_RAW   = 3;
		const int CAN_RAW    = 1;
		const int CAN_MTU    = 16;
		const int SIOCGIFMTU = 0x8921;
		const int SIOCSIFMTU = 0x8922;
		const int SIOCGIFINDEX = 0x8933;

		[StructLayout(LayoutKind.Sequential)]
		struct sockaddr_ll
		{
			public ushort sll_family;
			public ushort sll_protocol;
			public int sll_ifindex;
			public ushort sll_hatype;
			public byte sll_pkttype;
			public byte sll_halen;
			[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
			public byte[] sll_addr;
		}

		[StructLayout(LayoutKind.Sequential)]
		struct sockaddr_can
		{
			public ushort can_family;
			public int can_ifindex;
			public uint rx_id;
			public uint tx_id;
		}

		[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
		struct ifreq
		{
			public ifreq(string ifr_name)
			{
				this.ifr_name = ifr_name;
				this.ifr_ifindex = 0; // ifru_ivalue
				this.ifru_mtu = 0;
			}

			[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 16)]
			public string ifr_name;
			public uint   ifru_mtu;
			public int    ifr_ifindex;
		}

		[DllImport("libc", SetLastError = true)]
		private static extern int socket(int domain, int type, int protocol);

		[DllImport("libc", SetLastError = true)]
		private static extern int bind(int fd, ref sockaddr_can addr, int addrlen);

		[DllImport("libc", SetLastError = true)]
		private static extern int if_nametoindex(string ifname);

		[DllImport("libc", SetLastError = true)]
		private static extern int ioctl(int fd, int request, ref ifreq mtu);

#endregion

		public string Interface { get; set; }
		public int Baud { get; set; }
		public int Timeout { get; set; }
		public uint MinMTU { get; set; }
		public uint MaxMTU { get; set; }

		private static NLog.Logger logger = LogManager.GetCurrentClassLogger();
		protected override NLog.Logger Logger { get { return logger; } }

		private UnixStream _socket = null;
		private MemoryStream _recvBuffer = null;
		private int _bufferSize = 0;
		private uint _mtu = 0;
		private uint orig_mtu = 0;

		public CanPublisher(Dictionary<string, Variant> args)
			: base(args)
		{
		}

		protected override void OnOpen()
		{
			System.Diagnostics.Debug.Assert(_socket == null);

			_socket = OpenSocket(null);

			System.Diagnostics.Debug.Assert(_socket != null);
			System.Diagnostics.Debug.Assert(_bufferSize > 0);

			Logger.Debug("Opened interface \"{0}\" with MTU {1}.", Interface, _bufferSize);
		}

		private UnixStream OpenSocket(uint? mtu)
		{
			/*
			sockaddr_ll sa = new sockaddr_ll();
			sa.sll_family = AF_CAN;
			sa.sll_protocol = CAN_RAW;
			sa.sll_ifindex = if_nametoindex(Interface);
			*/
			sockaddr_can addr = new sockaddr_can();
			addr.can_family = AF_CAN;

			int fd = -1, ret = -1;

			try
			{
				fd = socket(AF_CAN, SOCK_RAW, CAN_RAW);
				UnixMarshal.ThrowExceptionForLastErrorIf(fd);

				ifreq ifr = new ifreq(Interface);
				/* Doesn't seem to work
				ret = ioctl(fd, SIOCGIFINDEX, ref ifr);
				UnixMarshal.ThrowExceptionForLastErrorIf(ret);
				*/
				addr.can_ifindex = if_nametoindex(Interface);

				if (addr.can_ifindex == 0)
					throw new ArgumentException("The interface \"" + Interface + "\" is not valid.");

				ret = bind(fd, ref addr, Marshal.SizeOf(addr));
				UnixMarshal.ThrowExceptionForLastErrorIf(ret);

/*
				if (orig_mtu == 0)
				{
					ret = ioctl(fd, SIOCGIFMTU, ref ifr);
					UnixMarshal.ThrowExceptionForLastErrorIf(ret);
					orig_mtu = ifr.ifru_mtu;
				}

				if (mtu != null)
				{
					ifr.ifru_mtu = mtu.Value;
					//ret = ioctl(fd, SIOCSIFMTU, ref ifr);
					ret = ioctl(fd, SIOCGIFMTU, ref ifr);
					UnixMarshal.ThrowExceptionForLastErrorIf(ret);
				}

*/
				ret = ioctl(fd, SIOCGIFMTU, ref ifr);
				UnixMarshal.ThrowExceptionForLastErrorIf(ret);
/*
				if (mtu != null && ifr.ifru_mtu != mtu.Value)
					throw new PeachException("MTU change did not take effect.");
*/

				_mtu = ifr.ifru_mtu;
/*

				if (ifr.ifru_mtu > (MaxMTU - EthernetHeaderSize))
					_bufferSize = (int)MaxMTU;
				else
					_bufferSize = (int)(ifr.ifru_mtu + EthernetHeaderSize);
*/
				_bufferSize = CAN_MTU;

				var stream = new UnixStream(fd);

				fd = -1;

				return stream;
			}
			catch (InvalidOperationException ex)
			{
				if (ex.InnerException != null)
				{
					var inner = ex.InnerException as UnixIOException;
					if (inner != null && inner.ErrorCode == Errno.EPERM)
						throw new PeachException("Access denied when opening the raw ethernet publisher.  Ensure the user has the appropriate permissions.", ex);
				}

				throw;
			}
			finally
			{
				if (fd != -1)
					Syscall.close(fd);
			}
		}

		protected override void OnClose()
		{
			//this never happens....
			System.Diagnostics.Debug.Assert(_socket != null);
			if (orig_mtu != 0)
				OpenSocket(orig_mtu);

			_socket.Close();
			_socket = null;
		}

		protected override void OnStop()
		{
			if (orig_mtu != 0)
			  OpenSocket(orig_mtu);
		}


		protected override void OnInput()
		{
			System.Diagnostics.Debug.Assert(_socket != null);

			if (_recvBuffer == null || _recvBuffer.Capacity < _bufferSize)
				_recvBuffer = new MemoryStream(_bufferSize);

			_recvBuffer.Seek(0, SeekOrigin.Begin);
			_recvBuffer.SetLength(_recvBuffer.Capacity);

			byte[] buf = _recvBuffer.GetBuffer();
			int offset = (int)_recvBuffer.Position;
			int size = (int)_recvBuffer.Length;

			Pollfd[] fds = new Pollfd[1];
			fds[0].fd = _socket.Handle;
			fds[0].events = PollEvents.POLLIN;

			int expires = Environment.TickCount + Timeout;
			int wait = 0;

			for (;;)
			{
				try
				{
					wait = Math.Max(0, expires - Environment.TickCount);
					fds[0].revents = 0;

					int ret = Syscall.poll(fds, wait);

					if (UnixMarshal.ShouldRetrySyscall(ret))
						continue;

					UnixMarshal.ThrowExceptionForLastErrorIf(ret);

					if (ret == 0)
						throw new TimeoutException();

					if (ret != 1 || (fds[0].revents & PollEvents.POLLIN) == 0)
						continue;

					var rxLen = _socket.Read(buf, offset, size);
					
					_recvBuffer.SetLength(rxLen);

					if (Logger.IsDebugEnabled)
						Logger.Debug("\n\n" + Utilities.HexDump(_recvBuffer));

					// Got a valid packet
					return;
				}
				catch (Exception ex)
				{
					Logger.Error("Unable to receive CAN packet on {0}. {1}", Interface, ex.Message);

					throw new SoftException(ex);
				}
			}
		}

		protected override void OnOutput(BitwiseStream data)
		{
			if (Logger.IsDebugEnabled)
				Logger.Debug("\n\n" + Utilities.HexDump(data));

			long count = data.Length;
			//var buffer = new byte[MaxMTU];
			var buffer = new byte[CAN_MTU];
			int size = data.Read(buffer, 0, buffer.Length);

			Pollfd[] fds = new Pollfd[1];
			fds[0].fd = _socket.Handle;
			fds[0].events = PollEvents.POLLOUT;

			int expires = Environment.TickCount + Timeout;
			int wait = 0;

			for (;;)
			{
				try
				{
					wait = Math.Max(0, expires - Environment.TickCount);
					fds[0].revents = 0;

					int ret = Syscall.poll(fds, wait);

					if (UnixMarshal.ShouldRetrySyscall(ret))
						continue;

					UnixMarshal.ThrowExceptionForLastErrorIf(ret);

					if (ret == 0)
						throw new TimeoutException();

					if (ret != 1 || (fds[0].revents & PollEvents.POLLOUT) == 0)
						continue;

					//_socket.Write(buffer, 0, size);
					_socket.Write(buffer, 0, CAN_MTU);

					if (count != size)
						throw new Exception(string.Format("Only sent {0} of {1} byte packet.", size, count));

					return;
				}
				catch (Exception ex)
				{
					Logger.Error("Unable to send CAN packet to {0}. {1}", Interface, ex.Message);

					throw new SoftException(ex);
				}
			}
		}

		#region Read Stream

		public override bool CanRead
		{
			get { return _recvBuffer.CanRead; }
		}

		public override bool CanSeek
		{
			get { return _recvBuffer.CanSeek; }
		}

		public override long Length
		{
			get { return _recvBuffer.Length; }
		}

		public override long Position
		{
			get { return _recvBuffer.Position; }
			set { _recvBuffer.Position = value; }
		}

		public override long Seek(long offset, SeekOrigin origin)
		{
			return _recvBuffer.Seek(offset, origin);
		}

		public override int Read(byte[] buffer, int offset, int count)
		{
			return _recvBuffer.Read(buffer, offset, count);
		}

		protected override Variant OnGetProperty(string property)
		{
			if (property == "MTU")
			{
				if (_socket != null)
				{
					Logger.Debug("MTU of {0} is {1}.", Interface, _mtu);
					return new Variant(_mtu);
				}

				using (var sock = OpenSocket(null))
				{
					Logger.Debug("MTU of {0} is {1}.", Interface, _mtu);
					return new Variant(_mtu);
				}
			}

			return null;
		}

		protected override void OnSetProperty(string property, Variant value)
		{
			if (property == "MTU")
			{
				uint mtu = 0;

				if (value.GetVariantType() == Variant.VariantType.BitStream)
				{
					var bs = (BitwiseStream)value;
					bs.SeekBits(0, SeekOrigin.Begin);
					ulong bits;
					int len = bs.ReadBits(out bits, 32);
					mtu = Endian.Little.GetUInt32(bits, len);
				}
				else if (value.GetVariantType() == Variant.VariantType.ByteString)
				{
					byte[] buf = (byte[])value;
					int len = Math.Min(buf.Length * 8, 32);
					mtu = Endian.Little.GetUInt32(buf, len);
				}
				else
				{
					throw new SoftException("Can't set MTU, 'value' is an unsupported type.");
				}

				if (MaxMTU >= mtu && mtu >= MinMTU)
				{
					try
					{
						using (var sock = OpenSocket(mtu))
						{
							Logger.Debug("Changed MTU of {0} to {1}.", Interface, mtu);
						}
					}
					catch (Exception ex)
					{
						string err = "Failed to change MTU of '{0}' to {1}. {2}".Fmt(Interface, mtu, ex.Message);
						Logger.Error(err);
						var se = new SoftException(err, ex);
						throw new SoftException(se);
					}
				}
			}
		}

		#endregion
	}
}
