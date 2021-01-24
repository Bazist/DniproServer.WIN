﻿/*
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
    public enum ChangeEnum
    {
        Add,
        UpdateRecursively,
        UpdateOnlyTopLevel,
        DeleteExistingItem
    };

    public class Change
    {
        private Change()
        {
        }

        public static Change Create(object childObject,
                                    ChangeEnum changeType = ChangeEnum.UpdateRecursively)
        {
            return new Change()
            {
                ChildObject = childObject,
                ChangeType = changeType
            };
        }

        public static Change[] CreateOne(object childObject,
                                        ChangeEnum changeType = ChangeEnum.UpdateRecursively)
        {
            return new Change[] { Change.Create(childObject, changeType) };
        }

        public object ChildObject { get; set; }
        public ChangeEnum ChangeType { get; set; }
    };
}
