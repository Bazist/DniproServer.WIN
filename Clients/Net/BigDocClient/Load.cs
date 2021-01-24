/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: dniprodb@gmail.com, skype: vyacheslavm81)
# This file is part of DniproClient.
#
# DniproClient is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# DniproClient is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DniproClient
{
    public enum LoadEnum
    {
        LoadRecursively,
        LoadOnlyTopLevel
    };

    public class Load
    {
        private Load()
        {
        }

        public static Load Create(object childObject,
                                  LoadEnum loadType = LoadEnum.LoadRecursively,
                                  uint index = 0)
        {
            return new Load()
            {
                ChildObject = childObject,
                LoadType = loadType,
                Index = index
            };
        }

        public static Load[] CreateOne(object childObject,
                                        LoadEnum loadType = LoadEnum.LoadRecursively,
                                        uint index = 0)
        {
            return new Load[] { Create(childObject, loadType, index) };
        }

        public object ChildObject { get; set; }
        public LoadEnum LoadType { get; set; }
        public uint Index { get; set; }
    };
}
