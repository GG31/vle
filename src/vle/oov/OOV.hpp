/** 
 * @file OOV.hpp
 * @brief 
 * @author The vle Development Team
 * @date 2007-08-23
 */

/*
 * Copyright (C) 2007 - The vle Development Team
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef VLE_OOV_OOV_HPP
#define VLE_OOV_OOV_HPP

#include <ostream>

namespace vle { namespace oov {

    class OOV
    {
    public:
        /** 
         * @brief Print the informations of VLE framework.
         * 
         * @param out stream to put information.
         */
        static void printInformations(std::ostream& out);

        /** 
         * @brief Print the version of VLE framework.
         * 
         * @param out stream to put information.
         */
        static void printVersion(std::ostream& out);
    };

}} // namespace vle oov 

#endif