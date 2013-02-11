/*
 * The contents of this file are subject to the terms
 * of the Common Development and Distribution License
 * (the "License").  You may not use this file except
 * in compliance with the License.
 *
 * You can obtain a copy of the license at
 * http://www.opensource.org/licenses/cddl1.php
 * See the License for the specific language governing
 * permissions and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL
 * HEADER in each file and include the License file at
 * http://www.opensource.org/licenses/cddl1.php.  If
 * applicable, add the following below this CDDL HEADER,
 * with the fields enclosed by brackets "[]" replaced
 * with your own identifying information:
 * Portions Copyright [yyyy]
 * [name of copyright owner]
 */

/*
 * $(@)__REPLACE__ $Revision: 1.3 $ $Date: 2006/09/20 01:16:29 $
 *
 * Copyright 2006 Sun Microsystems, Inc. All Rights Reserved.
 */

package org.omc.dream.mmi.common;

import java.io.Reader;
import java.io.StringReader;

public class MMIPlainTextParser extends MMIParser {
	boolean started;
	MMIPlainTextParserLogic mmiPlainTextParserLogic;

	public MMIPlainTextParser() {
		started = false;
		mmiPlainTextParserLogic = null;
	}
	
	public MMIMessage parseMessage(Reader r) throws ParseException,InvalidMMIObjectException {
		if(started) {
			return mmiPlainTextParserLogic.parseMessage(r);
		} else {
			mmiPlainTextParserLogic = new MMIPlainTextParserLogic(r);
			started = true;
			return mmiPlainTextParserLogic.parseMessage(null);
		}
	}	
        
        public MMIMessage parseMessage(String str) throws ParseException, InvalidMMIObjectException {
                return parseMessage(new StringReader(str));
        }
}
