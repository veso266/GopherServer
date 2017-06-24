﻿using GopherServer.Core.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GopherServer.Core.Results
{
    public class ByteResult : BaseResult
    {
        public ByteResult()
        {

        }

        public ByteResult(byte[] resultBytes, ItemType type)
        {
            this.ItemType = type;
            this.ResultBytes = resultBytes;
        }

        public byte[] ResultBytes { get; set; }

        public override void WriteResult(Stream stream)
        {
            using (var writer = new BinaryWriter(stream))
            {
                writer.Write(this.ResultBytes);
            }
        }
    }
}
